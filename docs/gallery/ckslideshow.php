<?php
/* 
CKSlideShow copyleft 2006 Chris Korda
derived from PHPSlideShow by Greg Lawler
This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation.
01	21mar13	changed deprecated $HTTP_GET_VARS to $_GET
*/
// get URL arguments
$curPic = $_GET['curPic'];
$folder = $_GET['folder'];
$title = $_GET['title'];
$home = $_GET['home'];
$delay = $_GET['delay'];
$auto = $_GET['auto'];
if ($folder == "")
	$folder = ".";
if ($delay == "")
	$delay = "3";	// default pause in seconds
if ($home == "")
	$home = "/";
// find image names
$dh = opendir($folder);
$picName = array();
while ($file = readdir($dh)) {
	if (preg_match("/(jpg|jpeg|gif|png)$/i", $file)) {
		$picName[] = "$folder/$file";
	}
}
$pics = count($picName);
sort($picName);
// set indices
if ($curPic == "")
	$curPic = 0;
$nextPic = $curPic + 1;
if ($nextPic >= $pics)
	$nextPic = 0;
$prevPic = $curPic - 1;
if ($prevPic < 0)
	$prevPic = pics - 1;
// create URLs
$baseUrl = "ckslideshow.php?folder=$folder&delay=$delay&title=$title&home=$home";
$baseUrl = str_replace(" ", "%20", $baseUrl);	// replace spaces with %20
$curUrl = "$baseUrl&curPic=$curPic";
$nextUrl = "$baseUrl&curPic=$nextPic";
$prevUrl = "$baseUrl&curPic=$prevPic";
// output HTML
echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
echo "<html>\n";
echo "<head>\n";
echo "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";
if ($auto)
	echo "<meta http-equiv=\"refresh\" content=\"$delay;url=$nextUrl&auto=1\">\n";
echo "<title>$title</title>\n";
echo "<link href=\"ckslideshow.css\" rel=stylesheet type=text/css>";
echo "<script type=\"text/javascript\">\n";
echo "<!--\n";
echo "if (document.images) { pic1 = new Image(640, 480); pic1.src=\"$picName[$nextPic]\" }\n"; 
echo "// -->\n";
echo "</script>\n";
echo "</head>\n";
echo "<body>\n";
echo "<a href=\"$nextUrl\"><img class=thumb src=\"$picName[$curPic]\" alt=\"image\" title=\"click for next image...\"></a>\n";
echo "<div class=\"navtext\">\n";
if ($curPic > 0)
	echo "<a href=\"$prevUrl\">back</a>\n";
$picNumber = $curPic + 1;
echo "[$picNumber of $pics]\n";
if ($curPic < $pics - 1)
	echo "<a href=\"$nextUrl\">next</a>\n";
if ($auto)
	echo "| <a href=\"$curUrl\">stop slideshow</a>\n";
else
	echo "| <a href=\"$curUrl&auto=1\">start slideshow</a>\n";
echo "| <a href=\"$baseUrl\">beginning</a>\n";
echo "| <a href=\"$home\">home</a>\n";
echo "</div>\n";
echo "</body>\n";
echo "</html>\n";
?>
