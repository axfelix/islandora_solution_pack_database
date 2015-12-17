<?php

/**
 * @file
 * This file contains all admin functions.
 * Of which there are not a lot at the moment.
 */

/**
 * Admin form function.
 */
function islandora_database_admin($form, &$form_state) {

  $form = array();

  // AJAX wrapper for url checking.
  $form['islandora_database_url_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('TEXT'),
  );

  $form['islandora_database_url_fieldset']['wrapper'] = array(
    '#prefix' => '<div id="islandora-url">',
    '#suffix' => '</div>',
    '#type' => 'markup',

/**
 * Admin form validate function.
 */
function islandora_database_admin_validate($form, &$form_state) {
}
