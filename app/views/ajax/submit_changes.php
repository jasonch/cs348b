<?php

  require('../../ajax/functions.php');
  require('functions.php');
  $result = array();
  $result["error"] = 0;

  if (!authenticate($_POST['pass'])) {
    $result["error"] = 1; // authenticate error
    $result["error_msg"] = "Authenticate Error";
  } else if (empty($_POST['title']) || empty($_POST['last_modified'])) {
    $result["error"] = 3; // input errors
    $result["error_msg"] = "No Page Selected";
  } else {

    $db = get_db();
    $content = $_POST['content'];
    //$content = $db->mysql_escape($_POST['content']);

    // somehow check to see if someone else have updated before me
    // doesn't work yet, need to pass in the last_modified time pulled by the last request
    $query = "SELECT 1 FROM 2012_static_pages WHERE title='{$_POST['title']}' AND last_modified='{$_POST['last_modified']}' ";
    $success = $db->query($query);
    if ($db->resultCount() == 0) {
      $result["error"] = 4;
      $result["error_msg"] = "The page has been modified while you are making changes. You might need to start over.";
    } else {

      // ready to update the page
	    $query = "UPDATE 2012_static_pages SET content = '{$content}', last_modified = CURRENT_TIMESTAMP WHERE title='{$_POST['title']}'";
	    $success = $db->query($query);
	    if ($success && $db->affectedCount() == 1) {
	      $result["title"] = $_POST['title'];
	      $result["content"] = $content;
	
	    } else {
	      $result["error"] = 2; // db error
	      $result["error_msg"] = $db->showErrors(); 
	    }
    }
  }
  echo json_encode($result);
?>
