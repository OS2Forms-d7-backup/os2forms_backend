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
 * Create tree for webform submissions
 * 
 */
function _os2forms_server_communication_build_component_tree(array &$elements, $key_parent, $key_element_id, $key_data, $parentId = 0) {
    $branch = array();

    foreach ($elements as $element) {
        if ($element[$key_parent] == $parentId) {
            $children = _os2forms_server_communication_build_component_tree($elements, $key_parent, $key_element_id, $key_data, $element[$key_element_id]);

            if ($children) {
               foreach($children as $key => $value) {
                  unset($value[$key_parent]);
                  unset($value[$key_element_id]);
                   if (array_key_exists($key_data, $value)) {
                      $value=$value[$key_data];
                    }

                 $element[$key] = $value;
               }


            }
            $branch[$element[$key_element_id]] = $element;

        }
    }
    return $branch;
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
  $group_fields = array('bt_layout_row', 'bt_layout_cols', 'fieldset');
  $group_fields_ids = array();
  $parent_elements = array();
  //bdpm(buildTree($node->webform['components']));
  foreach ($node->webform['components'] as $key => $component) {
    if ($component["pid"] == 0 ) {
      $parent_elements[] = $component["form_key"];
    }
    if (in_array($component['type'], $group_fields)) {
      $group_fields_ids[$key] = $component["form_key"];
    }
  }
  $result= array();

  foreach ($node->webform['components'] as $key => $component) {
    if (in_array($component['type'], $text_like_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]]['data'] = "";
      } else {
        $data[$component["form_key"]]['data'] = "[slettet]";
      }
      $data[$component["form_key"]]['parent_id'] =  $group_fields_ids[$component["pid"]];
    $data[$component["form_key"]]['cid'] =  $component["form_key"];
    }
     elseif (in_array($component['type'],  $group_fields)) {
      $data[$component["form_key"]]['parent_id'] = $group_fields_ids[$component["pid"]];
      $data[$component["form_key"]]['cid'] =  $component["form_key"];
    }
    elseif (in_array($component['type'], $nullable_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]]['data'] = NULL;
        $data[$component["form_key"]]['parent_id'] =  $group_fields_ids[$component["pid"]];
        $data[$component["form_key"]]['cid'] =  $component["form_key"];
      }


    }
    elseif (in_array($component['type'], $number_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]]['data'] = "";
      } else {
        $data[$component["form_key"]]['data'] = "0";
      }
      $data[$component["form_key"]]['parent_id'] =  $group_fields_ids[$component["pid"]];
    $data[$component["form_key"]]['cid'] =  $component["form_key"];
    }
    elseif (in_array($component['type'], $email_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]]['data'] = "";
      } else {
        $data[$component["form_key"]]['data'] = "slettet@slettet.dk";
      }
      $data[$component["form_key"]]['parent_id'] =  $group_fields_ids[$component["pid"]];
    $data[$component["form_key"]]['cid'] =  $component["form_key"];
    }
    elseif (in_array($component['type'], $cpr_fields)) {
      if (!$component["required"]) {
        $data[$component["form_key"]]['data'] = "";
      } else {
        $data[$component["form_key"]]['data'] = "0000000000";
      }
      $data[$component["form_key"]]['parent_id'] =  $group_fields_ids[$component["pid"]];
      $data[$component["form_key"]]['cid'] =  $component["form_key"];
    }
  }


  $data = _os2forms_server_communication_build_component_tree($data, 'parent_id', 'cid', 'data');
  foreach($data as $key =>$value){
    if(!in_array($key, $parent_elements)) {
      unset($data[$key]);
    }
    else {
      unset($data[$key]['cid']);
      unset($data[$key]['parent_id']);
      if (array_key_exists('data', $value)) {
        $data[$key] = $value['data'];
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
