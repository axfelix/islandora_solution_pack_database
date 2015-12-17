<?php

/**
 * @file
 * Handles the uploading of the database file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the database file.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_database_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('SQL');
  $form = array();
  $form['file'] = array(
    '#title' => t('Database Import'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );

  return $form;
}

/**
 * Addes the uploaded file into the ingestable objects 'OBJ' datastream.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_database_upload_form_submit(array $form, array &$form_state) {
  $object = $form_state['islandora']['objects'][0];
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }
  $database_file = file_load($form_state['values']['file']);
  $database_path = drupal_realpath($database_file->uri);
  $ds->setContentFromFile($database_path, FALSE);
  $ds->label = $database_file->filename;
  $ds->mimetype = $database_file->filemime;
}
