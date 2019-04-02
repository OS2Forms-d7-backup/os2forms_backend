<?php

use Os2formsServerCommunication\RemoteServer;
use Os2formsServerCommunication\RemoteServerLogger;

define("OS2FORMS_SERVER_COMMUNICATION_MAX_SUBSEQ_WS_REQUESTS", 10);

module_load_include('inc', 'webform', 'includes/webform.submissions');

/**
 * Performs a synch process for remote servers.
 */
function os2forms_server_communication_sync_submissions() {
  $servers = os2forms_server_communication_get_servers();

  foreach ($servers as $server) {
    RemoteServerLogger::info('Server: %s Synching session started', $server->server_name);

    $submissionsToSynched = 0;
    $submissionsToSynch = 0;

    // Getting all unsynched submissions from that server.
    $remoteServer = new RemoteServer($server->url, $server->username, decrypt($server->psw));

    // Allowing to get limited number of webform submissions.
    // Limit = OS2FORMS_SERVER_COMMUNICATION_MAX_SUBSEQ_WS_REQUESTS * pageSize
    // (default to 100).
    for ($i = 0; $i < OS2FORMS_SERVER_COMMUNICATION_MAX_SUBSEQ_WS_REQUESTS; $i++) {
      $remoteSubmissions = $remoteServer->fetchSubmissions($i);

      // If this page returned no results, or the only result is the checksum.
      // No need to proceed further.
      if (empty($remoteSubmissions)) {
        break;
      }
      // Function cache to store remotely loaded webforms.
      $remoteWebforms = array();

      // Getting submission data for each submission.
      foreach($remoteSubmissions as $submission) {
        $submissionsToSynch++;
        $remoteWebform = NULL;
        $remoteFileComponents = array();
        $webform = NULL;

        // If remote webform is not loaded yet, load remote webform.
        if (array_key_exists($submission->webform_uuid, $remoteWebforms)) {
          $remoteWebform = $remoteWebforms[$submission->webform_uuid];
        }
        else {
          $webformRemoteExport = $remoteServer->getWebformByUuid($submission->webform_uuid);

          if (!$webformRemoteExport) {
            // Could not export node.
            RemoteServerLogger::warning('Could not export webform uuid: %s. Skipping submission', $submission->webform_uuid);
            continue;
          }

          // Using node_export to read the node, but not saving it yet.
          $import_result = node_export_import($webformRemoteExport, $msg_t = 't', FALSE);
          if ($import_result['success']) {
            $remoteWebform = $import_result['nodes'][0];
            $remoteWebforms[$remoteWebform->uuid] = $remoteWebform;
          }
          else {
            // Could not import node.
            RemoteServerLogger::error('Could not import webform uuid: %s. Skipping submission', $submission->webform_uuid);
            continue;
          }
        }

        // Load local webform.
        $webforms = entity_uuid_load('node', array($submission->webform_uuid));
        if (!empty($webforms)) {
          $webform = reset($webforms);
        }

        // If local webform is missing or is outdated, create/update local webform.
        if (!$webform || $webform->changed != $remoteWebform->changed) {
          RemoteServerLogger::info('Creating/updating local node %s uuid: %s.', $remoteWebform->title, $remoteWebform->uuid);

          // Using node_export to actually create/update local webform.
          $import_result = node_export_import($webformRemoteExport);
          if ($import_result['success']) {
            $webform = node_load($import_result['nids'][0]);
          }
          else {
            // Could not import node.
            RemoteServerLogger::error('Could not import webform %s uuid: %s. Skipping submission', $remoteWebform->title, $submission->webform_uuid);
            continue;
          }
        }

        // Load submission data from remote.
        $submissionLoaded = $remoteServer->getSubmission($submission->webform_nid, $submission->sid);
        if (!$submissionLoaded || empty($submissionLoaded)) {
          // Could not load submission.
          RemoteServerLogger::error('Could not export submission uuid: %s, webform %s, uuid: %s. Skipping submission', $submission->uuid, $remoteWebform->title, $submission->webform_uuid);
          continue;
        }

        // Loading remote files.
        $fileCids = _os2forms_server_communication_get_file_cids($webform->nid);
        $fileImportError = FALSE;
        foreach ($fileCids as $fileCid) {
          if ($fileImportError) {
            break;
          }
          $remoteFids = $submissionLoaded->data->{$fileCid};

          if (!empty($remoteFids)) {
            foreach ($remoteFids as $delta => $remoteFid) {
              $remoteFile = $remoteServer->getFile($remoteFid);
              if (!$remoteFile || !isset($remoteFile->file)) {
                // Could not fetch remote file.
                RemoteServerLogger::error('Could not fetch remote file (fid: %d) for submission uuid: %s, webform %s, uuid: %s. Skipping submission', $remoteFid, $submission->uuid, $remoteWebform->title, $submission->webform_uuid);
                $fileImportError = TRUE;
                break;
              }

              $localFile = file_save_data(base64_decode($remoteFile->file), $remoteFile->uri);
              if (!$localFile || !isset($remoteFile->fid)) {
                // Could not store file locally.
                RemoteServerLogger::error('Could not store remote file (fid: %d) locally for submission uuid: %s, webform %s, uuid: %s. Skipping submission', $remoteFid, $submission->uuid, $remoteWebform->title, $submission->webform_uuid);
                $fileImportError = TRUE;
                break;
              }

              $remoteFileComponents[$fileCid][] = array(
                'delta' => $delta,
                'remote_fid' => $remoteFid,
                'local_fid' => $localFile->fid,
                'local_file' => $localFile,
              );
            }
          }
        }
        if ($fileImportError) {
          continue;
        }

        // Store submission data locally.
        if ($localSid = os2forms_server_communication_create_submission($webform, $submissionLoaded, $remoteFileComponents)) {
          $submissionsToSynched++;

          // Updating file usage table.
          foreach($remoteFileComponents as $fileComponent) {
            foreach ($fileComponent as $file) {
              file_usage_add($file['local_file'], 'webform', 'submission', $localSid);
            }
          }

          // Mark submissions as synched.
          if (!$remoteServer->getMarkSubmissionSynched($submissionLoaded->uuid)) {
            RemoteServerLogger::error('Could not mark submission as synched sid: %s, uuid: %s, webform: %s (nid: %d)', $submission->sid, $submission->uuid, $webform->title, $webform->nid);
          }
        }
      }
    }

    RemoteServerLogger::info('Server: %s Synching session completed, synched: %d / %d submission', $server->server_name, $submissionsToSynched, $submissionsToSynch);
  }
}

/**
 * Returns the list of remote servers with server ID as key.
 *
 * @return mixed
 *   List of servers mapped by server ID.
 */
function os2forms_server_communication_get_servers() {
  $query = db_select('os2forms_server_communication_servers_list', 's');
  $query->fields('s', array('id', 'server_name', 'url', 'username', 'psw'));
  $servers = $query->execute()
    ->fetchAllAssoc('id');

  return $servers;
}

/**
 * Creates a webform submission.
 *
 * Does it by copying the data from remotely loaded webform submission.
 * If system already has a submission with the same UUID, it will be deleted
 * first.
 *
 * @param mixed $webform
 *   Local webform node.
 * @param mixed $submission
 *   Remote webform submission, which is not saved locally.
 * @param mixed $remoteFileComponents
 *   Array of mapping between remote files if the local copies of them.
 *
 * @return int
 *   The new submission SID.
 */
function os2forms_server_communication_create_submission($webform, $submission, $remoteFileComponents) {
  // If we already have an old version of this submission, delete it first.
  $old_subission = webform_submission_uuid_submission($submission->uuid);
  if ($old_subission) {
    webform_submission_delete($webform, $old_subission);
  }

  $submission_data = $submission->data;

  foreach ($remoteFileComponents as $cid => $fileComponent) {
    foreach($fileComponent as $file) {
      if ($submission_data->{$cid}[$file['delta']] == $file['remote_fid']) {
        $submission_data->{$cid}[$file['delta']] = $file['local_fid'];
      }
    }
  }

  $submission = (object) array(
    'uuid' => $submission->uuid,
    'nid' => $submission->nid,
    'uid' => 0,
    'submitted' => $submission->submitted,
    'remote_addr' => $submission->remote_addr,
    'is_draft' => FALSE,
    'data' => (array) $submission_data,
  );
  return webform_submission_insert($webform, $submission);
}

/**
 * Returns the file components ids of a given webform.
 *
 * @param $nid
 *   Nid of the form.
 *
 * @return mixed
 *   Array of the file components id.
 */
function _os2forms_server_communication_get_file_cids($nid) {
  $query = db_select('webform_component', 'w')
    ->fields('w', array('cid'))
    ->condition('w.nid', $nid)
    ->condition('w.type', 'file');

  return $query->execute()->fetchCol();
}
