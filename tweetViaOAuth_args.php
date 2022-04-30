<?php
 
require_once('twitteroauth.php');  /* NOTE: This file, in-turn, requires OAuth.php */

$ckey = $argv[1];
$csecret = $argv[2];
$oauthtoken = $argv[3];
$oauthtokensecret = $argv[4];
$statusMsgText = $argv[5];

/* Twitter API Methods:  http://apiwiki.twitter.com/Twitter-API-Documentation  */

/* Setup a connection object for Twitter for the user/application defined in the global variables above...
 * Note, this does not make an actual connection or do any actual authenticating, at this stage! */
function setupAuthenticationObject()
	{
	echo "\nSetting up authentication object... ";
	global $ckey, $csecret, $oauthtoken, $oauthtokensecret;					/* declare these global variables to this function, so we can use them locally here */
	$objAuth = new TwitterOAuth ($ckey, $csecret, $oauthtoken, $oauthtokensecret);		/* create an authentication object */
	//$objAuth->format = 'xml';								/* specify use of XML instead of JSON */
	$objAuth->format = 'json';								/* specify use of XML instead of JSON */
	if($objAuth)										/* if object now exists... */
		{
		echo "SUCCESS\n";									/* complete the status message */
		return $objAuth;									/* return the authentication object */
		}
	else											/* else object does not exist... */
		{
		echo "FAILED\n";									/* complete the status message */
		return false;										/* return a testable false */
		}
	}

/* Post (tweet) the specified status-text string */
function postStatus($statusMsgText)
	{
	$objAuth = setupAuthenticationObject();							/* attempt to bring in an authentication object */
	if($objAuth)										/* if authentication object creation was successful, then continue... */
		{
		echo "Attempting to connect to Twitter and post status...\n";				/* write a status message */
		$response = $objAuth->post('statuses/update', array('status' => $statusMsgText));	/* attempt to connect and post the status to Twitter */
		return $response;									/* return the response from Twitter */
		}
	else											/* else object does not exist... */
		{
		echo "Connection could not be authenticated... Now exiting\n";				/* write a status message */
		return false;										/* return a testable false */
		}
	}

/* Get the tweet, per the specified status ID number string */ 
function getStatus($statusID)
	{
	$objAuth = setupAuthenticationObject();							/* attempt to bring in an authentication object */
	$response = $objAuth->get('statuses/show/'.$statusID);					/* get the status from Twitter */
	return $response;									/* return the response from Twitter */
	}

/* Get the most recent tweets (20?) of the authenticated user */
function getUserTimeline()
	{
	$objAuth = setupAuthenticationObject();							/* attempt to bring in an authentication object */
	$response = $objAuth->get('statuses/user_timeline');					/* get the status from Twitter */
	return $response;									/* return the response from Twitter */
	}

$response = postStatus($statusMsgText);

//echo "\n".var_dump($response)."\n";	//DEBUG PRINT

$fp = fopen("/tmp/twitterResponse.txt", "w");
if(array_key_exists('id', $response)) {
	fwrite($fp, "<text> = ".$response->text."\n<id> = ".$response->id);
}
else if(array_key_exists('errors', $response)) {
	fwrite($fp, "<error> = ".$response->errors[0]->message);
}
fclose($fp);

?>
