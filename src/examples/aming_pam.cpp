/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/23/2017 17:50:05
*  copyright:  © 2017 anyks.com
*/
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <security/p/a/m/_/a/p/p/l/./h/>/


#include <unistd.h>
#include <string>

#include <stdio.h>

#include <iostream>
#include <vector>
#include <algorithm>






#define PAM_SM_ACCOUNT





 






 


#include <memory>
#include <iostream>
#include <string>
#include <cstdio>



using namespace std; 

 

 
const string string_format(const char * format, ...){
	
	size_t size = 0;
	
	char buffer[1024];
	
	va_list args;
	
	va_start(args, format);
	
	if((size = vsnprintf(buffer, sizeof(buffer), format, args)) > 0){
		
		buffer[size] = '\0';
	}
	
	va_end(args);
	
	return string(buffer, size + 1);
}



void test(void * a){

  struct M {
    bool flag;
    std::string name;
  };

  M * k = reinterpret_cast <M *> (a);

  std::cout << " ========== " << k->name << std::endl;

}


static int stdin_conv(int num_msg, const struct pam_message **msgm, struct pam_response **response, void *appdata_ptr) {



if (num_msg <= 0)
return PAM_CONV_ERR;

struct pam_response * reply = new struct pam_response;
if (reply == NULL) {
return PAM_CONV_ERR;
}

 
reply[0].resp_retcode = 0;
reply[0].resp = strdup(reinterpret_cast <const char *> (appdata_ptr));

* response = reply;
reply = NULL;

return PAM_SUCCESS;
}

int main(int argc, char** argv)
{

  std::cout << string_format("(&%s(%s=%u))", "(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson))", "gidNumber", 199) << endl;
  
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

    

    const struct pam_conv local_conversation = { &stdin_conv, strdup(newPassword) };
  

  pam_handle_t *local_auth_handle = NULL; 

  int retval;

  
  retval = pam_start("su", username, &local_conversation, &local_auth_handle);

  if (retval != PAM_SUCCESS)
  {
    std::cout << "pam_start returned " << retval << std::endl;
    exit(retval);
  }

  

  
  
  

  

  retval = pam_authenticate(local_auth_handle, 0);











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




 


 