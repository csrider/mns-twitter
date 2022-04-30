/**********************************************************************
**  Module:     smtwitter.c
**
**		test with   smtwitter -twitter_account NAME -twitter_message "test"
**
**  Author:     (Redacted), Chris Rider
**          Copyright (c) 1991-2021
**
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>


/***** REDACTED *****/


#include "smtwitter.h"
#include "timeout.h"

/* globals needed to link */
int error_number = 0;
int dt_wtc_source;
int chain_yesno = 0;

char dt_rec_type[2];
char dt_range_type[2];
char dt_recno[RECORD_LENGTH];

char *db_date;
char *db_tabl;


/***** REDACTED *****/


FM_WHOLEMSG fm_wholemsgdata;

#define MAX_TWITTER_STR	(140)

/***********************************************************************
** void inthandler(int sig)
**
***********************************************************************/
void inthandler(int sig)
{
DiagnosticOverrideDataCall(1);

DIAGNOSTIC_LOG_1("SMTWITTER stopping due to signal (%d)", sig);

db_wtc_close();
db_hard_close();
db_signs_close();
db_bann_close();
db_syspa_close();

DiagnosticOverrideDataCall(-1);

exit(1);
}

/***********************************************************************
** void twitter_report_status_to_smbanner(int error_or_ok)
**
**	Report page status back to smbanner.
**
***********************************************************************/
void twitter_report_status_to_smbanner(int error_or_ok)
{
DB_WTC local_wtc;

memcpy(&local_wtc, db_wtc, sizeof(DB_WTC));			/* save wtc */

db_wtc->dwc_flag = error_or_ok;
db_wtc->dwc_operation = 0;
db_wtc->dwc_hard_recno = db_hard_getcur();
command_wtc(WTC_WRITE, WTC_SNPP_PAGE_STATUS, WTC_TWITTER, WTC_BANNER_BOARD, 0, 0);

memcpy(db_wtc, &local_wtc, sizeof(DB_WTC));			/* restore wtc */
}

/***********************************************************************
** void int_file_size_handler(int sig)
**
***********************************************************************/
void int_file_size_handler(int sig)
{
DiagnosticOverrideDataCall(1);

DIAGNOSTIC_LOG_1("smtwitter file size signal (%d)", sig);
SignalTrimlog();

DiagnosticOverrideDataCall(-1);
}

/***********************************************************************
** static void remove_quotes(char *str)
**
***********************************************************************/
static void remove_quotes(char *str)
{
int i;

char *ptr;
char characters_to_remove[] = { '"', '`', 0};

for(i = 0; characters_to_remove[i]; i++)
	{
	do
		{
		ptr = strchr(str, characters_to_remove[i]);
		if(ptr)
			{
			lm_st_delch(ptr, 0);
			}
		}
	while(ptr);
	}
}

/***********************************************************************
** int main(int argc,  char *argv[])
**
***********************************************************************/
int main(int argc,  char *argv[])
{
int i;
int next;
int found;
int debug = FALSE;
int twitter_error;
int smtwitter_timeout;
int more_message;
int file_size;
int last_size;
int test_twitter = FALSE;

char system_name[NODENAME_LENGTH];
char read_command[10000];
char system_command[2000];
char twitter_account[200];
char current_dtsec[DTSEC_LENGTH];
char test_twitter_message[1000] = "";
char test_twitter_account[50] = "";

char pid_str[PID_LENGTH];
char windows_pid_str[PID_LENGTH];

char buf[100];

guon_or_lasy = PRODUCT_SMTWITTER;		/* twitter product */

#ifdef USE_SMDATABASE
smdatabase_init();
#endif

for(i = 0; i < argc; i++)
	{
	if(!strcmp(argv[i], "-company"))
		{
		char name[TIMESHARE_COMPANY_NAME_LENGTH];

		i = ArgNext(i, argc, argv, name, TIMESHARE_COMPANY_NAME_LENGTH);
		TimeShareCompanyNameSet(name);
		}
	else if(!strcmp(argv[i], "-twitter_account"))
		{
		i = ArgNext(i, argc, argv, test_twitter_account, sizeof(test_twitter_account));
		test_twitter = TRUE;
		}
	else if(!strcmp(argv[i], "-twitter_message"))
		{
		i = ArgNext(i, argc, argv, test_twitter_message, sizeof(test_twitter_message));
		test_twitter = TRUE;
		}
	else if(!strcmp(argv[i], "-debug"))
		{
		debug = TRUE;
		}
	else if(!strcmp(argv[i], "-virtual_pid"))
		{
		char tmp[50];

		i = ArgNext(i, argc, argv, tmp, sizeof(tmp));
		virtual_pid = atoi(tmp);
		}
	}

SystemLookupTimeZone(FALSE);

SignalUnblockAllSignals();

system_get_name(system_name, NODENAME_LENGTH, NULL, 0, FALSE);
sprintf(buf, "%s/smtwitter.%s%s", SILENTM_LOG_DIRECTORY, system_name, SILENTM_LOG_SUFFIX);
DiagnosticLogFileSet(buf);

smtwitter_timeout = configuration_lookup_integer_min_max(DB_PATH_LOCATION, "TWITTER_TIMEOUT", 30, 10, 60);

DIAGNOSTIC_LOG_1("smtwitter version %s ", SILENT_MESSENGER_VERSION);
DIAGNOSTIC_LOG_1("%s", COPYRIGHT);

SignalActionCatchOnce(SIGTERM, inthandler);
SignalActionCatchOnce(SIGINT, inthandler);
SignalActionCatchRestart(SIGXFSZ, int_file_size_handler);

if(db_syspa_init()
	|| db_wtc_init()
	|| db_hard_init()
	|| db_signs_init()
	|| db_bann_init())
	{
	/* error already reported */
	return(1);
	}
else
	{
	wtc_format_pid(pid_str, PID_LENGTH, virtual_pid);
	wtc_format_pid(windows_pid_str, PID_LENGTH, 0);

	if(test_twitter)
		{
		twitter_error = -1;
		goto twitter_start_test;
		}

	while(1)
		{
twitter_continue:
		found = FALSE;
		more_message = FALSE;

		twitter_error = -1;

		db_wtc_select(6);			/* key with rec_type, pid, windows pid, priority, dtsec */

		/***** REDACTED *****/

		next = db_wtc_find();

twitter_next_check:
		get_dtsec(current_dtsec);

		if(next > 0
			&& !strcmp(db_wtc->dwc_pid, pid_str)
			&& !strcmp(db_wtc->dwc_windows_pid, windows_pid_str)
			&& !strcmp(db_wtc->dwc_rec_type, wtc_commands[WTC_TWITTER_PAGE]))
	        	{
			if(AlphaToDTSEC(db_wtc->dwc_dtsec) > AlphaToDTSEC(current_dtsec))
				{
				/* this message is delayed at this time */
				/* but continue to check the others */
				next = db_wtc_next();

				goto twitter_next_check;
				}
	        	else if(db_hard_setcur(db_wtc->dwc_parent_recno) > 0
	        		&& db_bann_setcur(db_wtc->dwc_record_num) > 0)
	        		{
				int page_retry = 0;
				int page_retry_max = 5;
				int character_limit = db_wtc->dwc_flag;

				char consumer_key[200] = "";
				char consumer_secret[200] = "";
				char access_token[200] = "";
				char access_token_secret[200] = "";
				char twitter_data[10000];
				char twitter_filename[FILENAME_LENGTH];

				if(character_limit > MAX_TWITTER_STR)
					{
					character_limit = MAX_TWITTER_STR - 1;
					}
				else if(character_limit == 0)
					{
					character_limit = MAX_TWITTER_STR - 1;
					}

				strcpyl(twitter_account, db_hard->dhc_epage, sizeof(twitter_account));
				remove_trailing_white_space(twitter_account);
				remove_trailing_white_space(db_wtc->dwc_msg_buffer);
				remove_quotes(db_wtc->dwc_msg_buffer);

twitter_start_test:
				if(test_twitter)
					{
					page_retry = 0;
					page_retry_max = 5;
					character_limit = sizeof(test_twitter_message);
					strcpyl(twitter_account, test_twitter_account, sizeof(twitter_account));
					strcpyl(db_wtc->dwc_data, "testing", sizeof(db_wtc->dwc_data));
					strcpyl(db_wtc->dwc_msg_buffer, test_twitter_message, sizeof(db_wtc->dwc_msg_buffer));
					}

				mn_snprintf(twitter_filename, sizeof(twitter_filename), "/etc/silentm/twitter.%s.txt", twitter_account);
				if(SystemReadDataFromFile(twitter_data, sizeof(twitter_data), twitter_filename) < 0)
					{
					DIAGNOSTIC_LOG_1("Twitter error no account file %s", twitter_filename);

					twitter_error = -1;
					}
				else
					{
					extract_after_str(twitter_data, access_token, sizeof(access_token), "Access Token: ");
					str_truncate_at_character(access_token, '\n');
					remove_trailing_white_space(access_token);

					extract_after_str(twitter_data, access_token_secret, sizeof(access_token_secret), "Access Token Secret: ");
					str_truncate_at_character(access_token_secret, '\n');
					remove_trailing_white_space(access_token_secret);

					extract_after_str(twitter_data, consumer_key, sizeof(consumer_key), "Consumer Key: ");
					str_truncate_at_character(consumer_key, '\n');
					remove_trailing_white_space(consumer_key);

					extract_after_str(twitter_data, consumer_secret, sizeof(consumer_secret), "Consumer Secret: ");
					str_truncate_at_character(consumer_secret, '\n');
					remove_trailing_white_space(consumer_secret);
					}

				for(page_retry = 0; twitter_error < 0 && page_retry < page_retry_max; page_retry++)
					{
					if(notjustspace(db_wtc->dwc_msg_buffer, sizeof(db_wtc->dwc_msg_buffer)) == FALSE)
						{
						DIAGNOSTIC_LOG_1("twitter error pin=%s message is blank.", db_wtc->dwc_data);

						twitter_error = -1;
						break;
						}
					else
						{
						int twitter_timeout = 30;

						char *twitter_ptr = db_wtc->dwc_msg_buffer;
						char twitter_str[MAX_TWITTER_STR];

						do
							{
							strcpyl(twitter_str, twitter_ptr, sizeof(twitter_str));
							twitter_str[character_limit] = 0;

							unlink("/tmp/twitterResponse.txt");
	
							mn_snprintf(system_command, sizeof(system_command), "/usr/bin/php -f /home/silentm/twitter/tweetViaOAuth_args.php \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" > /dev/null & ", consumer_key, consumer_secret, access_token, access_token_secret, twitter_str);

							if(debug)
								{
								DIAGNOSTIC_LOG_1("%s", system_command);
								SystemSendDataToFile("ok", 3, "/tmp/twitterResponse.txt", FALSE);
								twitter_error = 0;
								}
							else if(SystemCheckFileExists("/usr/bin/php", "") < 0)
								{
								DIAGNOSTIC_LOG("smtwitter: /usr/bin/php is not installed.");
								}
							else
								{
								twitter_error = system(system_command);

								if(test_twitter)
									{
									sleep(1);
									SystemReadDataFromFile(read_command, sizeof(read_command), "/tmp/twitterResponse.txt");
									unlink("/tmp/twitterResponse.txt");

									printf("%s\n", read_command);

									goto twitter_done;
									}
								}
	
							file_size = -1;
							TimeoutSetTimeout(twitter_timeout);
							do
								{
								last_size = file_size;
								file_size = SystemReadDataFromFile(read_command, sizeof(read_command), "/tmp/twitterResponse.txt");

 								if(TimeoutCheckTimeoutUsingDTSEC())
									{
									int again_seconds = 5;

									DIAGNOSTIC_LOG_6("Twitter [retry=%d/%d] timeout at %d seconds, do again in %d. pin=%s msg='%s'", db_wtc->dwc_baudrate + 1, page_retry_max, twitter_timeout, again_seconds, db_wtc->dwc_data, db_wtc->dwc_msg_buffer);
									file_size = 1;

									get_dtsec_plus_offset(db_wtc->dwc_dtsec, again_seconds);
									db_wtc->dwc_baudrate++;					/* bad attempt count */
									db_wtc_write();

									if(db_wtc->dwc_baudrate >= page_retry_max)
										{
										more_message = FALSE;
										twitter_error = -1;
										goto twitter_done;
										}

									goto twitter_continue;
									}
								else if(file_size < 0 || (file_size != last_size))
									{
									mn_delay(500);
									}
								}
							while(file_size < 0 || (file_size != last_size));

							if(file_size > 0
								&& strstr(read_command, "<id>"))
								{
								twitter_error = 0;

								DIAGNOSTIC_LOG_2("Twitter message OK pin=%s msg='%s'", db_wtc->dwc_data, twitter_str);

								twitter_ptr += strlen(twitter_str);
								if(strlen(twitter_ptr) > 0)
									{
									/* still more characters to send */
									strcpyl(wtc_msg_buffer, twitter_ptr, sizeof(wtc_msg_buffer));
									strcpyl(db_wtc->dwc_msg_buffer, wtc_msg_buffer, sizeof(db_wtc->dwc_msg_buffer));
									
									/* update the dtsec as we cannot send again for 60 seconds */
									get_dtsec_plus_offset(db_wtc->dwc_dtsec, 61);
									db_wtc_write();

									more_message = TRUE;
									}

								twitter_ptr = NULL;
								}
							else
								{
								/* add the original command to the error file to assist in diagnostics */
								SystemSendDataToFile("\n\n", 2, "/tmp/twitterResponse.txt", TRUE);
								SystemSendDataToFile(system_command, strlen(system_command), "/tmp/twitterResponse.txt", TRUE);
								SystemSendDataToFile("\n\n", 2, "/tmp/twitterResponse.txt", TRUE);
								SystemSendDataToFile(read_command, strlen(read_command), "/tmp/twitterResponse.txt", TRUE);
	
								if(stristr(read_command, "over capacity"))
									{
	        							found = FALSE;
									DIAGNOSTIC_LOG_2("Twitter temporary timeout, trying again. pin=%s msg='%s'", db_wtc->dwc_data, db_wtc->dwc_msg_buffer);

									mn_delay(2*1000);

									goto twitter_continue;
									}
								else if(stristr(read_command, "could not authenticate"))
									{
	        							found = FALSE;
									DIAGNOSTIC_LOG_2("Twitter could not authenticate (removing), pin=%s msg='%s'", db_wtc->dwc_data, db_wtc->dwc_msg_buffer);

									db_wtc_delete();

									goto twitter_continue;
									}
								else if(stristr(read_command, "Status is a duplicate"))
									{
	        							found = FALSE;
									DIAGNOSTIC_LOG_2("Twitter duplicate message (removing). pin=%s msg='%s'", db_wtc->dwc_data, db_wtc->dwc_msg_buffer);

									db_wtc_delete();

									goto twitter_continue;
									}
								else if(stristr(read_command, "<error>"))
									{
	        							found = FALSE;
									DIAGNOSTIC_LOG_3("Twitter error (%s). pin=%s msg='%s'", read_command, db_wtc->dwc_data, db_wtc->dwc_msg_buffer);

									db_wtc_delete();

									goto twitter_continue;
									}

								twitter_error = -1;
								twitter_ptr = NULL;
								}
							}
						while(twitter_ptr);
						}

					if(twitter_error < 0)
						{
						DIAGNOSTIC_LOG_4("Twitter [retry=%d/%d] pin=%s msg='%s'", page_retry + 1, page_retry_max, db_wtc->dwc_data, db_wtc->dwc_msg_buffer);
						mn_delay(1*1000);
						}
					}
			        }
			else
				{
				DIAGNOSTIC_LOG_3("twitter error with db_bann_setcur for '%s' '%s' '%s'", db_wtc->dwc_data, db_wtc->dwc_guon_table, db_wtc->dwc_msg_buffer);
				}

twitter_done:
			if(test_twitter)
				{
				break;
				}
			else if(more_message)
				{
				/* still more message to send so dont remove the record */
				}
			else
				{
			        /***** REDACTED *****/
				}

			/***** REDACTED *****/
	
	        	found = TRUE;
			}

		if(found == FALSE)
			{
			/* no commands found wait a while */
			mn_delay(5*1000);
			}
	    }

	/***** REDACTED *****/

	}

return(0);
}
