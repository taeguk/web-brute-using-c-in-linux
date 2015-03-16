<?php
@session_start();
$msg='No message';
$max_range = 10000;
$admin_pw = $max_range;
if(!empty($_SESSION['islogin']) && $_SESSION['islogin'] == true && !empty($_GET['action']) && $_GET['action'] == "logout")
{
	$_SESSION['islogin'] = false;
	$msg = "Logout";
}
if(!empty($_POST['id']) && !empty($_POST['pw']))
{
	$id = $_POST['id'];
	$pw = $_POST['pw'];
	$msg = "asdfsadf";

	if($id == 'guest' && $pw == '1500')
	{
		$_SESSION['islogin'] = true;
		$_SESSION['id'] = $id;
		$msg = "Login Success";
	}
	else if($id == 'admin' && $pw == $admin_pw)
	{
		$_SESSION['islogin'] = true;
		$_SESSION['id'] = $id;
		$msg = "Login Success";
	}
	else
	{
		$msg = "Login Fail";
	}
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
	<head>
		<title>Web Brute Force Test</title>
		<meta http-equiv="Content-Type" content="text/html" charset="utf-8">
		<meta name="viewport" content="width=device-width">
		<style type="text/css">
		</style>
		<script type="text/javascript">
		</script>
	</head>
	<body>
		<p>Objective : Login to admin (Hint : guest/1234, password is number(0 - <?php echo $max_range; ?>))</p>
		<?php echo $msg; ?><br/>
<?php
	if(!empty($_SESSION['islogin']) && $_SESSION['islogin'] == true)
	{
?>
		You are <?php echo $_SESSION['id']; ?><br/>
		<a href='<?php echo $_SERVER['PHP_SELF'] . "?action=logout" ?>'>Logout</a>
<?php
	}
	else
	{
?>
		<form method='post' action='<?php echo $_SERVER['PHP_SELF']; ?>'>
			ID : <input name='id' type='text' size='20' /><br/>
			PW : <input name='pw' type='password' size='20' /><br/>
			<input name='submit' type='submit' value='Login'/>
		</form>
<?php
	}
?>
	</body>
</html>
