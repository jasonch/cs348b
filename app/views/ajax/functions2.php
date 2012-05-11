<?

require('../ajax/globals.php');
require('db.php');

function get_db()
{
  return new DB(DB_USER, DB_PASSWORD, DB_NAME, DB_HOST);
}

?>
