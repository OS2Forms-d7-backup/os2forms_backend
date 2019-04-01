<?php

/**
 * @file
 * os2forms_backend.monolog.inc
 */

/**
 * Implements hook_default_monolog_profiles().
 */
function os2forms_backend_default_monolog_profiles() {
  $export = array();

  $profile = new stdClass();
  $profile->disabled = FALSE; /* Edit this to true to make a default profile disabled initially */
  $profile->api_version = 1;
  $profile->name = 'os2forms_server_communication_pr';
  $profile->options = array(
    'handlers' => array(
      'rotating_file_handler' => array(
        'weight' => '1',
        'label' => 'Rotating File Handler',
        'level' => '200',
        'bubble' => '1',
        'filepath' => 'private://monolog/os2forms_server_communication.log',
        'max_files' => '30',
        'handler' => 'rotating_file',
      ),
    ),
    'label' => 'OS2Forms server communication profile',
  );
  $export['os2forms_server_communication_pr'] = $profile;

  return $export;
}
