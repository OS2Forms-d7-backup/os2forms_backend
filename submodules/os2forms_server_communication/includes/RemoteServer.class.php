<?php

namespace Os2formsServerCommunication;

/**
 * Handles the connection to remote webservice.
 */
class RemoteServer {
  private $endpoint;
  private $authToken;

  /**
   * Fetches the endpoint URL and puts it into a variable.
   */
  public function __construct($endpoint, $username, $password) {
    $this->endpoint = $endpoint;
    $this->authToken = 'Basic ' . base64_encode($username . ':' . $password);
  }

  /**
   * Calling webservice endpoint: GET /os2forms_submission.
   *
   * @param int $page
   *   Number of page to return.
   *
   * @return array
   *   Result array.
   */
  public function fetchSubmissions($page = 0) {
    $getParams = http_build_query(
      array(
        'page' => $page,
      )
    );
    $requestUrl = $this->endpoint . '/os2forms_submission.json?' . $getParams;
    $result = $this->requestWrapper($requestUrl);
    return $result;
  }

  /**
   * Calling webservice endpoint : DELETE /os2forms_submission/uuid.
   *
   * Marks submission as synched.
   *
   * @param string $uuid
   *   Uuid of the submission.
   *
   * @return int
   *   1 if operation is successful, 0 otherwise.
   */
  public function getMarkSubmissionSynched($uuid) {
    $options = array();
    $options['method'] = 'DELETE';

    $requestUrl = $this->endpoint . '/os2forms_submission/' . $uuid . '.json';

    $result = $this->requestWrapper($requestUrl, $options);

    if (!empty($result) && is_array($result)) {
      return array_pop($result);
    }
  }

  /**
   * Calling webservice endpoint: /webform_submission/nid/sid.
   *
   * @param string $nid
   *   nid of the webform.
   * @param string $sid
   *   sid of the submission.
   *
   * @return mixed
   *   Submission object.
   */
  public function getSubmission($nid, $sid) {
    $requestUrl = $this->endpoint . '/webform_submission/' . $nid . '/' . $sid . '.json';
    $result = $this->requestWrapper($requestUrl);

    return $result;
  }

  /**
   * Calling webservice endpoint: /file/fid.
   *
   * @param string $fid
   *   Fid of the file.
   *
   * @return mixed
   *   File object.
   */
  public function getFile($fid) {
    $requestUrl = $this->endpoint . '/file/' . $fid . '.json';
    $result = $this->requestWrapper($requestUrl);

    return $result;
  }

  /**
   * Calling webservice endpoint : GET /os2forms_webform/uuid.
   *
   * @param string $uuid
   *   Uuid of the webform.
   *
   * @return mixed
   *   Serialized webform formatted as string or FALSE is no data was returned.
   */
  public function getWebformByUuid($uuid) {
    $requestUrl = $this->endpoint . '/os2forms_webform/' . $uuid . '.json';
    $result = $this->requestWrapper($requestUrl);

    if (!empty($result) && is_array($result)) {
      $result = array_pop($result);
      if (!empty($result)) {
        return $result;
      }
      else {
        return FALSE;
      }
    }
  }

  /**
   * Helper function.
   *
   * Wraps the request, adds authorization string and performs
   * an HTTP request call.
   */
  private function requestWrapper($requestUrl, $options = array()) {
    $options['headers'] = array();
    $options['headers']['Authorization'] = $this->authToken;
    $options['headers']['Content-Type'] = 'application/json';
    $result = drupal_http_request($requestUrl, $options);

    if ($result->code == 200) {
      // Check if the string is in JSON format.
      if (is_string($result->data) && is_array(json_decode($result->data, TRUE))) {
        return json_decode($result->data);
      }
      else {
        return $result->data;
      }
    }
  }
}
