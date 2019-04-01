<?php

/**
 * @file
 * os2forms_backend.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function os2forms_backend_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "monolog" && $api == "monolog") {
    return array("version" => "1");
  }
}
