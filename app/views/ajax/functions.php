<?

  /* very simple authentication based on sha-1 */
  function authenticate($pass) {
    include('constants.php');
    return (!empty($pass) && sha1($pass) == ADMIN_PASSWORD_HASHED);
  }

?>
