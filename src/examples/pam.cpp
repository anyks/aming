#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <security/pam_appl.h>
#include <unistd.h>
#include <string>

#include <stdio.h>
#include <string.h>




// #include <syslog.h>

/*
CHECK_INCLUDE_FILES (security/pam_misc.h HAVE_SECURITY_PAM_MISC_H)
    CHECK_INCLUDE_FILES (security/openpam.h HAVE_SECURITY_OPENPAM_H)

    And change auth_pam_common.h to:

    #if HAVE_SECURITY_PAM_MISC_H
    #include <security/pam_misc.h>
    #elif HAVE_SECURITY_OPENPAM_H
    #include <security/openpam.h>
    #endif
 */



// To build this:
// g++ ./examples/pam.cpp -lpam -o ./bin/test

/*
struct pam_response *reply;


//function used to get user input
int function_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    *resp = reply;
    return PAM_SUCCESS;
}

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: check_user <username>\n");
        exit(1);
    }
    const char *username;
    username = argv[1];

    const struct pam_conv local_conversation = { function_conversation, NULL };
  pam_handle_t *local_auth_handle = NULL; // this gets set by pam_start

  int retval;

  // local_auth_handle gets set based on the service
  retval = pam_start("su", username, &local_conversation, &local_auth_handle);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_start returned " << retval << std::endl;
    exit(retval);
  }

  reply = (struct pam_response *)malloc(sizeof(struct pam_response));

  // *** Get the password by any method, or maybe it was passed into this function.
  reply[0].resp = getpass("Password: ");
  reply[0].resp_retcode = 0;

  //pam_set_item(local_auth_handle, PAM_AUTHTOK, reply);

  retval = pam_authenticate(local_auth_handle, 0);


// openlog("pamtest_aming", 0, LOG_LOCAL0);
// syslog(LOG_LOCAL1, "Can not open file \"%s\" for writing.", "pamtest_aming.conf");
// closelog();
// # more /var/log/messages
// # more /var/log/system.log




  if (retval != PAM_SUCCESS)
  {
    if (retval == PAM_AUTH_ERR)
    {
        std::cout << "Authentication failure." << std::endl;
    }
    else
    {
        std::cout << "pam_authenticate returned " << retval << std::endl;
    }
    exit(retval);
  }

  std::cout << "Authenticated." << std::endl;

  retval = pam_end(local_auth_handle, retval);

  free(reply);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_end returned " << retval << std::endl;
    exit(retval);
  }

  return retval;
}
*/



/*
struct pam_response * reply;


int function_conversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    * resp = reply;
    return PAM_SUCCESS;
}


int main(int argc, char** argv) {

 pam_handle_t * pamh;
 const struct pam_conv pamc = { function_conversation, NULL };
 
 const char *username = argv[1];
 

 // Начало сеанса аутентификации.
 pam_start("su", username, &pamc, &pamh);

 reply = new struct pam_response;

 // std::string password = "6991163"; // getpass("Password: ");
 std::string password = getpass("Password: ");

  // *** Get the password by any method, or maybe it was passed into this function.
  reply[0].resp = strdup(password.c_str());
  reply[0].resp_retcode = 0;

  int retval = pam_authenticate(pamh, 0);

  if (retval != PAM_SUCCESS)
  {
    if (retval == PAM_AUTH_ERR)
    {
        std::cout << "Авторизация не удалась." << std::endl;
    }
    else
    {
        std::cout << "Такой пользователь не существует " << retval << std::endl;
    }
  } else {
    std::cout << "Авторизация прошла успешно" << std::endl;
  }

 // Конец сеанса.
 retval = pam_end(pamh, retval);

 if (retval != PAM_SUCCESS)
 {
   std::cout << "Ошибка закрытия сессии авторизации " << retval << std::endl;
   exit(retval);
 }

 return retval;
}
*/




int
converse(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{
  struct pam_response *aresp;
  // char buf[PAM_MAX_RESP_SIZE];
  int i;

  aresp = new struct pam_response;

  for (i = 0; i < n; ++i) {
    aresp[i].resp_retcode = 0;
    aresp[i].resp = NULL;
    switch (msg[i]->msg_style) {
      case PAM_PROMPT_ECHO_OFF:
        aresp[i].resp = strdup(getpass(msg[i]->msg));
      break;
    }

    if (aresp[i].resp == NULL)  std::cout << " ---- NULL ---- " << std::endl;
    else break;
    // std::cout << " ---- " << msg[i]->msg << std::endl;
  }

  *resp = aresp;

  /*
    struct pam_response *aresp;
    char buf[PAM_MAX_RESP_SIZE];
    int i;

    data = data;
    if (n <= 0 || n > PAM_MAX_NUM_MSG)
        return (PAM_CONV_ERR);
    if ((aresp = calloc(n, sizeof *aresp)) == NULL)
        return (PAM_BUF_ERR);
    for (i = 0; i < n; ++i) {
        aresp[i].resp_retcode = 0;
        aresp[i].resp = NULL;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
            aresp[i].resp = strdup(getpass(msg[i]->msg));
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_PROMPT_ECHO_ON:
            fputs(msg[i]->msg, stderr);
            if (fgets(buf, sizeof buf, stdin) == NULL)
                goto fail;
            aresp[i].resp = strdup(buf);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_ERROR_MSG:
            fputs(msg[i]->msg, stderr);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stderr);
            break;
        case PAM_TEXT_INFO:
            fputs(msg[i]->msg, stdout);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stdout);
            break;
        default:
            goto fail;
        }
    }
    *resp = aresp;
    return (PAM_SUCCESS);
 fail:
        for (i = 0; i < n; ++i) {
                if (aresp[i].resp != NULL) {
                        memset(aresp[i].resp, 0, strlen(aresp[i].resp));
                        free(aresp[i].resp);
                }
        }
        memset(aresp, 0, n * sizeof *aresp);
    *resp = NULL;
    return (PAM_CONV_ERR);
    */

    return PAM_SUCCESS;
}

int main() {
 pam_handle_t* pamh;
 struct pam_conv pamc;
 // Указание диалоговой функции.
 pamc.conv = &converse;// &openpam_ttyconv; // &misc_conv;
 pamc.appdata_ptr = NULL;

 struct pam_conv *conv;
 struct pam_message msg;
 struct pam_response *resp;
 const struct pam_message *msgp;

 std::string password;

 static char password_prompt[] = "Password1:";

 int pam_err, retry;

 
 // Начало сеанса аутентификации.
 pam_start("su", getenv("USER"), &pamc, &pamh);


 pam_err = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
 
  if (pam_err != PAM_SUCCESS)
      return (PAM_SYSTEM_ERR);
  msg.msg_style = PAM_PROMPT_ECHO_OFF;
  msg.msg = password_prompt;
  msgp = &msg;
 
 
  pam_err = (*conv->conv)(1, &msgp, &resp, conv->appdata_ptr);
 
  if (resp != NULL) {
   if (pam_err == PAM_SUCCESS)
       password = resp->resp;
   else
       free(resp->resp);
   free(resp);
 }

 std::cout << " +++ " << password << std::endl;


 // Аутентификация пользователя.
 if (pam_authenticate(pamh, 0) != PAM_SUCCESS)
  fprintf(stderr, "Authentication failed!\n");
 else
  fprintf(stderr, "Authentication OK\n");

 // Конец сеанса.
 pam_end(pamh, 0);
 return 0;
}


