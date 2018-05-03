<?php

function os2forms_server_communication_settings_page() {
  $headers = array(
    array('data' => t('ID'), 'field' => 'id'),
    array('data' => t('Frontend server')),
    array('data' => t('URL')),
    array('data' => t('Edit')),
    array('data' => t('Delete'))
  );
  $query = db_select('os2forms_server_communication_servers_list', 's');
  $query->fields('s', array('id', 'server_name', 'url'));

  $result = $query->execute()
    ->fetchAllAssoc('id');

  $rows = array();

  foreach ($result as $row) {
    //$row[] = $edit;
    $edit = l(t('Edit'), '/admin/config/os2forms_server_communication/server/' . $row->id . '/edit');
    $delete = l(t('Delete'), '/admin/config/os2forms_server_communication/server/' . $row->id . '/delete');

    $rows[] = array_merge((array) $row, (array) $edit, (array) $delete);
  }

  $output = "<h3>" . t('Frontend servers') . "</h3>";
  $output .= '<ul class="action-links"> <li>'
    . l(t('Add'), '/admin/config/os2forms_server_communication/server/add', array('attributes' => array('name' => 'add_server')))
    . '</li>'
    //. '<li>' . l(t('Clear backend and all frontend server'), '/admin/config/os2forms_server_communication/submissions/clear', array('attributes' => array('name' => 'add_server')))
    . '</li></ul>';
  $output .= theme('table', array('header' => $headers, 'rows' => $rows));
  return $output;// . render(drupal_get_form('os2forms_server_communication_submissions_download_settings_form'));
}

/**
 * Implements hook_FORM_alter().
 */
function os2forms_server_communication_add_server_form($form, $form_state) {
  return os2forms_server_communication_server_form();
}

function os2forms_server_communication_delete_server_form($form, $form_state) {
  if (isset($form_state['input']['op']) && $form_state['input']['op'] == t('Cancel')) {
    drupal_goto('/admin/config/os2forms_server_communication/settings');
  }
  $server_id = arg(4);
  $query = db_select('os2forms_server_communication_servers_list', 's');
  $query->fields('s', array('server_name'));
  $query->condition('s.id', $server_id, '=');
  $result = $query->execute()
    ->fetchField();

  $form[] = array(
    '#prefix' => '<h3>',
    '#markup' => t('Are you sure you want to delete server %name?', array('%name' => $result)),
    '#suffix' => '</h3>',
  );
  $form[] = array(
    '#prefix' => '<p>',
    '#markup' => t('This action cannot be undone'),
    '#suffix' => '</p>',
  );

  $form['id'] = array(
    '#type' => 'hidden',
    '#default_value' => $server_id
  );
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete')
  );
  $form['cancel'] = array(
    '#type' => 'button',
    '#value' => t('Cancel')
  );
  return $form;
}

function os2forms_server_communication_delete_server_form_submit($form, $form_state) {
  $input = $form_state['input'];
  extract($input);
  db_delete('os2forms_server_communication_servers_list')
    ->condition('id', $id)
    ->execute();
  drupal_set_message(t('Server is deleted'), 'status');
  drupal_goto('/admin/config/os2forms_server_communication/settings');
}

function os2forms_server_communication_edit_server_form($form, $form_state, $server_id = NULL) {
  $server_id = arg(4);
  $query = db_select('os2forms_server_communication_servers_list', 's');
  $query->fields('s', array('id', 'server_name', 'url', 'username', 'psw'));
  $query->condition('s.id', $server_id, '=');
  $result = $query->execute()
    ->fetchObject();
  $form = os2forms_server_communication_server_form();
  $form["server_name"]['#default_value'] = $result->server_name;
  $form["server_url"]['#default_value'] = $result->url;
  $form["username"]['#default_value'] = $result->username;
  $form["password"]['#default_value'] = decrypt($result->psw);
  $form["id"]['#default_value'] = $result->id;
  return $form;
}

function os2forms_server_communication_server_form() {
  $form['server_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Server name:'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('Name of fronend server'),
  );
  $form['id'] = array(
    '#type' => 'hidden',
  );

  $form['server_url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL:'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('URL for server access'),
  );
  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('User name:'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('User authentificated on frontend server'),
  );
  $form['password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password:'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('Passwod for frontend server access'),
  );

  $form['add'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  return $form;
}

function os2forms_server_communication_add_server_form_submit($form, $form_state) {
  $input = $form_state['input'];
  extract($input);

  db_insert('os2forms_server_communication_servers_list')
    ->fields(array(
      'server_name' => $server_name,
      'url' => $server_url,
      'username' => $username,
      'psw' => encrypt($password),
    ))
    ->execute();

  drupal_set_message(t('Server added'), 'status');
  drupal_goto('/admin/config/os2forms_server_communication/settings');
}

function os2forms_server_communication_edit_server_form_submit($form, $form_state) {
  $input = $form_state['input'];
  extract($input);

  db_update('os2forms_server_communication_servers_list')
    ->fields(array(
      'server_name' => $server_name,
      'url' => $server_url,
      'username' => $username,
      'psw' => encrypt($password),
    ))
    ->condition('id', $id, '=')
    ->execute();

  drupal_set_message(t('Server modified'), 'status');
  drupal_goto('/admin/config/os2forms_server_communication/settings');
}
