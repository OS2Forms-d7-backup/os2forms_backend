<?php

/**
 * @file
 * Views hooks implemented for the Webform2pdf module.
 */

/**
 * Implements hook_views_data_alter().
 */
function os2forms_server_communication_views_data_alter(&$data){
  // View submission link.  
  $data['webform_submissions']['download_pdf_file'] = array(
    'title' => t('Download PDF'),
    'help' => t('Provide a simple link to pdf.'),
    'real field' => 'serial',
    'field' => array(
      'handler' => 'os2forms_server_communication_handler_field_submission_download_pdf_file',
      'click sortable' => TRUE,
      'real field' => 'sid',
      'link_type' => 'download pdf file',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
}

/**
 * Implements hook_views_default_views_alter().
 */
function os2forms_server_communication_views_default_views_alter(&$views) {
  if (isset($views['webform_submissions'])) {
    $handler =&$views['webform_submissions']->display['default']->handler;
    $download_pdf = array(
      'id' => 'download_pdf_file',
      'table' => 'webform_submissions',
      'field' => 'download_pdf_file',
      'label' => 'download pdf file',
      'element_label_colon' => FALSE,
      'access_check' => 0,
    );   
   $handler->display->display_options['fields']['download_pdf_file'] = $download_pdf;
   $handler->display->display_options['style_options']['columns']['download_pdf_file'] = 'view_submission_1';
  }
  
}