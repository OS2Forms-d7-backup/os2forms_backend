<?php

/**
 * @file
 * Os2forms_server_communication.drush.inc.
 */

/**
 * Implements hook_drush_command().
 */
function os2forms_server_communication_drush_command() {
  $items = array();
  $items['os2forms-server-communication-synch'] = array(
    'callback' => 'os2forms_server_communication_sync_submissions',
    'description' => 'Drush command perform OS2Forms server communication sync submissions process.',
    'aliases' => array('forms-ss'),
    'examples' => array(
      'Perform OS2Forms server communication sync submissions process' => 'drush os2forms-server-communication-synch',
      'Perform OS2Forms server communication sync submissions process (alias)' => 'drush forms-ss',
    ),
  );
  return $items;
}
