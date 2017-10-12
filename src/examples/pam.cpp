#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <security/pam_appl.h>
#include <unistd.h>

#include <syslog.h>

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
  retval = pam_start("common-auth", username, &local_conversation, &local_auth_handle);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_start returned " << retval << std::endl;
    exit(retval);
  }

  reply = (struct pam_response *)malloc(sizeof(struct pam_response));

  // *** Get the password by any method, or maybe it was passed into this function.
  reply[0].resp = getpass("Password: ");
  reply[0].resp_retcode = 0;

  retval = pam_authenticate(local_auth_handle, 0);

	openlog("pamtest_aming", 0, LOG_LOCAL0);
	syslog(LOG_LOCAL1, "Can not open file \"%s\" for writing.", "pamtest_aming.conf");
	closelog();
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

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_end returned " << retval << std::endl;
    exit(retval);
  }

  return retval;
}