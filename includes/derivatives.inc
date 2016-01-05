<?php

/**
 * @file
 * Contains all the functions required for creating live databases from SQL ingest.
 */

/**
 * Gets the source of all derivatives as an unmanaged file.
 *
 * @param AbstractObject $object
 *   The object to generate/add the deriviative datastreams to.
 *
 * @return string
 *   The file path to the unmanaged derivative source file.
 */
function islandora_database_get_derivative_source_as_unmanaged_file(AbstractObject $object) {
  $mime_detect = new MimeDetect();
  $ext = $mime_detect->getExtension($object['OBJ']->mimeType);
  $file_name = str_replace(':', '-', $object->id);
  $file_uri = file_create_filename("{$file_name}.OBJ.{$ext}", 'temporary://');
  $object['OBJ']->getContent($file_uri);
  return $file_uri;
}


/**
 * Writes a php wrapper around adminer that will automatically log in to a
 * given database with provided credentials
 *
 * @param string $username
 *   The auto-generated username for this new DB instance.
 * @param string $password
 *   The auto-generated password for this new DB instance.
 * @param string $database
 *   The name of the database (derived from SQL if possible).
 * @param string pid
 *	 The Fedora PID of the object we're making a DB wrapper for.
 * @param string $docker_ip
 *   The IP addres of the docker instance (defaults to localhost).
 * @param string $port
 *   The port that this DB is running on (defaults to 33060:3306).
 * @param string $adminer_path
 *   The path to adminer.php. Points to the version in SP by default.
 *
 * @return filename
 *   The file path to the new adminer wrapper.
 */
function islandora_database_generate_adminer_wrapper($username, $password, $database, $pid, $docker_ip, $port, $adminer_path) {
  $filename = "temporary://adminer_" . $pid . ".php";
  $docker_address = $docker_ip . ":" . $port;
  $adminer_wrapper = '<?php
  function adminer_object() {
      include_once "./plugins/plugin.php";
      foreach (glob("plugins/*.php") as ' . $filename . ') {
          include_once "./' . $filename . '";
      }
      $plugins = array(
          new AdminerFrames,
      );
      class AdminerCustomization extends AdminerPlugin {
        function credentials() {
          return array("' . $docker_address . '", "' . $username . '", "' . $password . '");
        }
        function database() {
          return "' . $database . '";
        }
      }
      return new AdminerCustomization($plugins);
  }
  include "' . $adminer_path . '";
  ?>';
  file_put_contents($filename, $adminer_wrapper);
  return $filename;
}


/**
 * Transforms SQL OBJ datastream to live DB instance.
 *
 * @param AbstractObject $object
 *   The original SQL dump OBJ datastream
 * @param bool $force
 *   Whether the derivative generation is being forced or not.
 *
 * @return array
 *   An array describing the outcome of the database startup
 *
 * @see hook_islandora_derivative()
 */
function islandora_database_create_all_derivatives(AbstractObject $object, $force = FALSE) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!isset($object['OBJ'])) {
    return islandora_database_missing_obj_datastream($object->id);
  }
  $file_uri = islandora_database_get_derivative_source_as_unmanaged_file($object);
  $source = drupal_realpath($file_uri);

  $port = 33060;
  $username = substr(md5(rand()), 0, 7);
  $password = substr(md5(rand()), 0, 7);
  $adminer_path = "/adminer-4.2.1-mysql.php";

  $docker_get_container = 'docker ps | grep 33060 | sed -e "s/\s.*//g"';
  exec($docker_get_container, $container_id);

  $docker_get_mysql = 'docker logs ' . implode($container_id) . ' | grep -oP "mysql -uadmin -p\S+"';
  # this writes some supervisord stuff to stderr in my environment but it's harmless
  exec($docker_get_mysql, $mysql_credentials);

  $docker_ip = "127.0.0.1";
  $pid = $object->id;

  $sql_connect_command = implode($mysql_credentials) . " -h127.0.0.1 -P" . $port . " --default-character-set=utf8";

  $module_path = drupal_get_path('module', 'islandora_database');
  $database_import_command = "bash " . $module_path . "/includes/import-database.sh " . $source . " " . $sql_connect_command . " " . $username . " " . $password;
  exec($database_import_command, $database);

  #preg_match('/(?<=CREATE DATABASE )\S+/', file_get_contents($file_uri), $database_scraped);
  #$database = preg_replace('/[\'"`;]/', '', array_values($database_scraped)[0]);

  $adminer_wrapper = islandora_database_generate_adminer_wrapper($username, $password, $database, $pid, $docker_ip, $port, $adminer_path);

  # copy the new adminer wrapper to the container
  $wrapper_to_container = "docker cp " . $adminer_wrapper . " " . implode($container_id) . ":/var/www/html/adminer_" . $pid . ".php";
  exec($wrapper_to_container, $execout, $success);

  file_unmanaged_delete($file_uri);

  if ($success) {
    return array(
      'success' => TRUE,
      'messages' => array(
        array(
          'message' => t('Created live database for @pid.'),
          'message_sub' => array(
            '@pid' => $object->id,
          ),
          'type' => 'dsm',
        ),
      ),
    );
  }
  if (!$success) {
    $success = array(
      'success' => FALSE,
      'messages' => array(
        array(
          'message' => t('`%command` returned an error when creating live database of PID %pid: %error'),
          'message_sub' => array(
            '%command' => $wrapper_to_container,
            '%pid' => $object->id,
            '%error' => $execout,
          ),
          'type' => 'dsm',
          'severity' => WATCHDOG_ERROR,
        ),
      ),
    );
    return array(
      'success' => FALSE,
      'messages' => array(
        array(
          'message' => t('Failed to create live database for @pid. Error message: @error'),
          'message_sub' => array(
            '@pid' => $object->id,
            '@error' => $success,
          ),
          'type' => 'watchdog',
          'severity' => WATCHDOG_ERROR,
        ),
      ),
    );
    }
  file_unmanaged_delete($temp);
  return $success;
}


// This is copied from Data Solution Pack for now.
function islandora_database_add_static_thumbnail(AbstractObject $object) {
  $path = drupal_get_path('module', 'islandora_database');
  $ds = $object->constructDatastream('TN', "M");
  $ds->label = 'TN';
  $ds->mimetype = 'image/png';
  $ds->setContentFromFile("$path/images/database.png");
  $object->ingestDatastream($ds);
}


/**
 * Adds the file as a datastream to the object using the DSID to identify it.
 *
 * @param AbstractObject $object
 *   The object to add the datastream to.
 * @param string $datastream_id
 *   The datastream id of the added datastream.
 * @param string $file_uri
 *   A URI to the file containing the content for the datastream.
 *
 * @return bool|string
 *   TRUE if successfully added/modified the datastream, error message if not.
 */
function islandora_database_add_datastream($object, $datastream_id, $file_uri) {
  $return = TRUE;
  try {
    $ingest = !isset($object[$datastream_id]);
    $mime_detector = new MimeDetect();
    if ($ingest) {
      $ds = $object->constructDatastream($datastream_id, "M");
      $ds->label = $datastream_id;
    }
    else {
      $ds = $object[$datastream_id];
    }
    $ds->mimetype = $mime_detector->getMimetype($file_uri);
    $ds->setContentFromFile(drupal_realpath($file_uri), FALSE);
    if ($ingest) {
      $object->ingestDatastream($ds);
    }
  }
  catch (exception $e) {
    $return = $e->getMessage();
  }
  file_unmanaged_delete($file_uri);
  return $return;
}


/**
 * Helper function declaring that no OBJ datastream is present.
 *
 * @param string $pid
 *   The pid of the ingested object.
 *
 * @return array
 *   An array describing the failure of the operation..
 *
 * @see hook_islandora_derivative()
 */
function islandora_database_missing_obj_datastream($pid) {
  return array(
    'success' => FALSE,
    'messages' => array(
      array(
        'message' => t('Could not ingest database %s. No valid database dump was uploaded.'),
        'message_sub' => array('%s' => $pid),
        'type' => 'watchdog',
        'severity' => WATCHDOG_ERROR,
      ),
    ),
  );
}
