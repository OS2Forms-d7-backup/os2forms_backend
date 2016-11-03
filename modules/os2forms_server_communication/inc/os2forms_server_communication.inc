<?php

/**
 * Returns the time when the server was last synched
 */
function os2forms_server_communication_last_synched($server_id) {
  $query = db_select('os2forms_server_communication_submission_update', 'os')
    ->fields('os', array('updated'))
    ->condition('os.frontend_server_id', $server_id, '=');
  $last_updated = $query->execute()->fetchField();

  return $last_updated;
}

/**
 * Returns the list of remote servers with server ID as key
 */
function os2forms_server_communication_get_servers() {
  $query = db_select('os2forms_server_communication_servers_list', 's');
  $query->fields('s', array('id', 'server_name', 'url', 'username', 'psw'));
  $servers = $query->execute()
    ->fetchAllAssoc('id');

  return $servers;
}

/**
 * Returns the list of webforms that exist on the server
 */
function os2forms_server_communication_get_webforms() {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'webform')
    ->propertyCondition('status', NODE_PUBLISHED);

  $result = $query->execute();

  if (isset($result['node'])) {
    $nids = array_keys($result['node']);
    $items = entity_load('node', $nids);
  }

  return $items;
}

/**
 * Copies and creates the actual submission on the backend server
 *
 * @return auto-generated unique submission_id
 */
function os2forms_server_communication_create_submission($nid, $data) {
  module_load_include('inc', 'webform', 'includes/webform.submissions');
  $submission_id = md5(uniqid(rand(), TRUE));
  $cid = (string) _os2forms_server_communication_get_cid_by_form_key($nid, "submission_id");
  $subm_data = (array) $data->data;
  foreach ($subm_data as $key => $value) {
    $pre_data[$key] = $value;
  }
  if (isset($pre_data[$cid])) {
    $pre_data[$cid][0] = $submission_id;
  }
  else {
    $pre_data[$cid] = array(0 => $submission_id);
  }

  $submission_data = $pre_data;

  global $user;

  $node = node_load($nid);
  $submission = (object) array(
    'nid' => $data->nid,
    'uid' => $user->uid,
    'submitted' => $data->submitted,
    'remote_addr' => $data->remote_addr,
    'is_draft' => FALSE,
    'data' => $submission_data,
  );
  $sid = webform_submission_insert($node, $submission);
  //TODO: _os2forms_server_communication_save_submission_pdf($nid, $sid);
  return $submission_id;
}

/*
 * Prepares the empty submission of a given form
 */
function _os2forms_server_communication_prepare_submission($nid) {
  $node = node_load($nid);
  $data = array();

  $text_like_fields = array('textfield', 'textarea', 'nemid_address', 'nemid_city', 'nemid_coaddress', 'nemid_name');
  $nullable_fields = array('select', 'date', 'date_popup');
  $number_fields = array('number', 'time');
  $email_fields = array('email');
  $cpr_fields = array('nemid_cpr');

  foreach ($node->webform['components'] as $key => $component) {
    if (in_array($component['type'], $text_like_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]] = "";
      } else {
        $data[$component["form_key"]] = "[slettet]";
      }
    }
    elseif (in_array($component['type'], $nullable_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]] = NULL;
      }
    }
    elseif (in_array($component['type'], $number_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]] = "";
      } else {
        $data[$component["form_key"]] = "0";
      }
    }
    elseif (in_array($component['type'], $email_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]] = "";
      } else {
        $data[$component["form_key"]] = "slettet@slettet.dk";
      }
    }
    elseif (in_array($component['type'], $cpr_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]] = "";
      } else {
        $data[$component["form_key"]] = "0000000000";
      }
    }
  }

  return $data;
}

/**
 * Returns the component id f the element based on form_key
 */
function _os2forms_server_communication_get_cid_by_form_key($nid, $form_key) {
  $query = db_select('webform_component', 'w')
    ->fields('w', array('cid'))
    ->condition('w.nid', $nid, '=')
    ->condition('w.form_key', $form_key, '=');
  return $query->execute()->fetchField();
}
