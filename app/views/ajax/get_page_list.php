<?php

  require('../../ajax/functions.php');
  require('functions.php');
  $result = array();
  $result["error"] = 0;

  if (!authenticate($_POST['pass'])) {
    $result["error"] = 1; // authenticate error
    $result["error_msg"] = "Authenticate Error";
    echo json_encode($result);
  } else {

    $db = get_db();
    $query = "SELECT * FROM 2012_static_pages";
    $success = $db->query($query);
    if ($success) {
      $first = true;
      $result["data"] = array();

      while(true) {
        $rows = $db->fetchRow();
        if (empty($rows)) break;
        $result["data"][] = $rows;
      }

    } else {
      $result["error"] = 2; // db error
      $result["error_msg"] = $db->showErrors(); 
    }
    echo json_encode($result);
  }
?>
