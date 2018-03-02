<?php
$topic = $_GET["topic"]; 
if (!$topic) {
$main = "Help/Getting_Started/Introduction.htm";
}
?>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Whorld Help</title>
<meta content="text/html; charset=windows-1252" http-equiv=Content-Type>
<link href="./style.css" rel=stylesheet title=default type=text/css>
</head>
<frameset cols="300,*">
  <frame src="Help/Contents.htm" name="toc">
  <frame src=<?php echo($topic); ?> name="topic">
</frameset>
