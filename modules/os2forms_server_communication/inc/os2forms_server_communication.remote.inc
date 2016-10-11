<?php

/*
 * Returns all submission from the remote server based on webform title
 *
 */
function os2forms_server_communication_remote_get_submissions($webform_title, $server)
{
    $options = array(
        'headers' => array(
            'Content-Type' => 'application/json',
            'Authorization' => "Basic " . base64_encode($server->username . ':' . decrypt($server->psw)),
        ),
        'method' => 'GET',
    );

    //getting webform nid
    $result = drupal_http_request($server->url . '/rest/node.json?parameters[type]=webform&parameters[title]=' . urlencode($webform_title), $options);
    if ($result->code != 200) {
        throw new Exception($result->status_message);
    }
    $result = json_decode($result->data);
    $webform_nid = $result[0]->nid;

    if ($webform_nid) {
        //getting submissions
        $result = drupal_http_request($server->url . '/rest/webform_submission.json?parameters[nid]=' . $webform_nid, $options);
        if ($result->code != 200) {
            throw new Exception($result->status_message);
        }
        $submissions = json_decode($result->data);
        return $submissions;
    }

    return false;
}

/**
 * Returns single submission data
 */
function os2forms_server_communication_remote_get_submission_data($server, $submission)
{
    $options = array(
        'headers' => array(
            'Content-Type' => 'application/json',
            'Authorization' => "Basic " . base64_encode($server->username . ':' . decrypt($server->psw)),
        ),
        'method' => 'GET',
    );

    $url = $server->url . '/rest/webform_submission/' . $submission->nid . '/' . $submission->sid . '.json';
    $new_submission = drupal_http_request($url, $options);
    if ($new_submission->code != 200) {
        throw new Exception($result->status_message);
    }
    return json_decode($new_submission->data);
}

/**
 * Updates the submission on the remote server
 */
function os2forms_server_communication_remote_update_submission($server, $submission, $data)
{
    $options = array(
        'headers' => array(
            'Content-Type' => 'application/json',
            'Authorization' => "Basic " . base64_encode($server->username . ':' . decrypt($server->psw)),
        ),
        'method' => "PUT",
        'data' => json_encode($data)
    );

    $url = $server->url . '/rest/webform_submission/' . $submission->nid . '/' . $submission->sid . '.json';
    $result = drupal_http_request($url, $options);

    if ($result->code != 200) {
        throw new Exception($result->status_message);
    }
}

/**
 * Deletes the submission on the remote server
 */
function os2forms_server_communication_remote_delete_submission($url, $headers)
{
    $options = Array(
        'headers' => $headers,
        'method' => "DELETE",
    );

    $result = drupal_http_request($url, $options);

    if ($result->code != 200) {
        watchdog('OS2forms', 'Request %uri was failed with message: %message', array('%uri' => $url, '%message' => $result->status_message), WATCHDOG_WARNING);
    }
}
