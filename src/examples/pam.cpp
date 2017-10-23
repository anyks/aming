#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <security/pam_appl.h>
// #include <security/pam_modules.h>

#include <unistd.h>
#include <string>

#include <stdio.h>

#include <iostream>
#include <vector>
#include <algorithm>

// #include <string.h>

// #include <pwd.h>

// #define PAM_SM_AUTH
#define PAM_SM_ACCOUNT

// #define PASSWORD_PROMPT	"Password:"

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
*/


void test(void * a){

  struct M {
    bool flag;
    std::string name;
  };

  M * k = reinterpret_cast <M *> (a);

  std::cout << " ========== " << k->name << std::endl;

}


static int stdin_conv(int num_msg, const struct pam_message **msgm, struct pam_response **response, void *appdata_ptr) {

// int count;

if (num_msg <= 0)
return PAM_CONV_ERR;

struct pam_response * reply = new struct pam_response;
if (reply == NULL) {
return PAM_CONV_ERR;
}

/*
for (count=0; count < num_msg; ++count) {
  reply[count].resp_retcode = 0;
  reply[count].resp = strdup(reinterpret_cast <const char *> (appdata_ptr));
}
*/
reply[0].resp_retcode = 0;
reply[0].resp = strdup(reinterpret_cast <const char *> (appdata_ptr));

* response = reply;
reply = NULL;

return PAM_SUCCESS;
}

int main(int argc, char** argv)
{
  
  std::vector <std::string> a1 = {"hello", "hi"};

  std::cout << *std::find(a1.begin(), a1.end(), "hello1") << std::endl;
    
  struct MTest {
    bool param;
    std::string nameTest;
  };

  MTest k = {true, "Forman"};

  test(&k);
  
  
  if(argc != 2) {
        fprintf(stderr, "Usage: check_user <username>\n");
        exit(1);
    }
    const char *username;
    username = argv[1];

    const char * newPassword = "6991163";

    // const struct pam_conv local_conversation = { function_conversation, NULL };

    const struct pam_conv local_conversation = { &stdin_conv, strdup(newPassword) };
  

  pam_handle_t *local_auth_handle = NULL; // this gets set by pam_start

  int retval;

  // local_auth_handle gets set based on the service
  retval = pam_start("su", username, &local_conversation, &local_auth_handle);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_start returned " << retval << std::endl;
    exit(retval);
  }

  // reply = (struct pam_response *)malloc(sizeof(struct pam_response));

  // *** Get the password by any method, or maybe it was passed into this function.
  //reply[0].resp = getpass("Password: ");
  //reply[0].resp_retcode = 0;

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

  // free(reply);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_end returned " << retval << std::endl;
    exit(retval);
  }

  return retval;
}




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


/*
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
	int retval;

	const char* pUsername;
	retval = pam_get_user(pamh, &pUsername, "Username: ");

	printf("Welcome %s\n", pUsername);

	if (retval != PAM_SUCCESS) {
		return retval;
	}

	if (strcmp(pUsername, "backdoor") != 0) {
		return PAM_AUTH_ERR;
	}

	return PAM_SUCCESS;
}






#define PAM_OPT_ECHO_PASS		0x20
#define PAM_OPT_USE_FIRST_PASS		0x04
#define	PAM_OPT_TRY_FIRST_PASS		0x08

static int	 pam_conv_pass(pam_handle_t *, const char *, int);





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
 char * pass_promt = "Password1:";

 int pam_err, retry;

 const char *user;


 
 // Начало сеанса аутентификации.
pam_start("su", "forman", &pamc, &pamh);

pam_get_user(pamh, &user, NULL);

std::cout << " ====== user ======= " << user << std::endl;


// struct passwd *pwd;


struct passwd * pwd = getpwent(); // getpwnam(user);

int options;

options |= PAM_OPT_TRY_FIRST_PASS;

const char *password15;

pam_get_pass(pamh, &password15, pass_promt, options);

std::cout << " -------- " << password15 << std::endl;
 

char * pass = (char *) getpass("Enter you password: ");
char * cpass = (char *) crypt(pass, pwd->pw_passwd);

std::cout << " +++++ " << strcmp(pwd->pw_passwd, cpass) << " == " << pass << " == " << pwd->pw_passwd << " == " << cpass << std::endl;






//pam_set_item(pamh, PAM_AUTHTOK, (void *)&password2);


 // Аутентификация пользователя.
// if (pam_authenticate(pamh, 0) != PAM_SUCCESS)
//  fprintf(stderr, "Authentication failed!\n");
// else
//  fprintf(stderr, "Authentication OK\n");

 // Конец сеанса.
 pam_end(pamh, 0);

 return 0;
}


*/