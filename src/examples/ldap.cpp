// $ g++ -std=c++11 -lldap ./examples/ldap.cpp -o ./bin/ldap
// clang++ -std=c++11 -L/usr/local/lib -lldap -DLDAP_DEPRECATED -I/usr/local/include ./examples/ldap.cpp -o ./bin/ldap
// FreeDSD: clang++ -std=c++11 -L/usr/local/lib -lldap -I/usr/local/include ./examples/ldap.cpp -o ./bin/ldap
// MacOS X: export MACOSX_DEPLOYMENT_TARGET="10.3"; clang++ -std=c++11 -O2 -Wall -fPIC -W -Waggregate-return -Wcast-align -Wmissing-prototypes -Wnested-externs -Wshadow -Wwrite-strings -ansi -L/usr/local/lib -lldap -I/usr/local/include ./examples/ldap.cpp -o ./bin/ldap

// $ ./bin/ldap
#include <stdio.h>
#include <ldap.h>
#define LDAP_SERVER "ldap://213.159.213.15:3892"

// ldapsearch -LL -u -t -v -h 213.159.213.15  -p 3892 -w 367895431279 -D "cn=admin,dc=agro24,dc=dev" -b "ou=users,dc=agro24,dc=dev" "(objectClass=inetOrgPerson)" uid forman

int main(int argc, char * argv[]){
    LDAP        *ld, *ld2;
    int         rc;
    char        bind_dn[100];
    LDAPMessage *result, *e;
    char *dn;
    //int has_value;

    sprintf( bind_dn, "cn=%s,dc=agro24,dc=dev", "admin" );
    printf( "Connecting as %s...\n", bind_dn );

    /*
    ld = ldap_init("213.159.213.15", 3892);

    if(ld == NULL){
        perror( "ldap_initialize" );
        return( 1 );
    }
    */
   
    
    if( ldap_initialize( &ld, LDAP_SERVER ) )
    {
        perror( "ldap_initialize" );
        return( 1 );
    }
    // LDAP_VERSION2

    int protocol_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
        return(1);
    }

    const char * password1 = "367895431279";

    #if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
      struct berval cred1 = { 0, NULL };
      cred1.bv_len = strlen(password1);
      cred1.bv_val = new char[cred1.bv_len + 1]; //malloc(cred.bv_len+1);
      strcpy(cred1.bv_val, password1);
      rc = ldap_sasl_bind_s (ld, bind_dn, LDAP_SASL_SIMPLE, &cred1, NULL, NULL, NULL);
      delete [] cred1.bv_val;
     // free(cred.bv_val);
      memset(&cred1, 0, sizeof(cred1));
    #else
      rc = ldap_simple_bind_s (ld, bind_dn, password1);
    #endif



    // rc = ldap_simple_bind_s( ld, bind_dn, "367895431279" );
    if( rc != LDAP_SUCCESS )
    {
        fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc) );
        return( 1 );
    }

    printf( "Successful authentication\n" );
    // LDAP_SCOPE_SUBTREE = sub, LDAP_SCOPE_ONELEVEL = one, LDAP_SCOPE_BASE = base
    // &(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson)(uid=forman)
    rc = ldap_search_ext_s(ld, "ou=users,dc=agro24,dc=dev", LDAP_SCOPE_SUBTREE, "(&(!(agro24CoJpDismissed=TRUE))(objectClass=inetOrgPerson)(uid=forman))", NULL, 0, NULL, NULL, NULL, 0, &result);
    if ( rc != LDAP_SUCCESS ) {
        fprintf(stderr, "ldap_search_ext_s: %s\n", ldap_err2string(rc));
    }

    for ( e = ldap_first_entry( ld, result ); e != NULL; e = ldap_next_entry( ld, e ) ) {
        
        if ( (dn = ldap_get_dn( ld, e )) != NULL ) {
            printf( "dn: %s\n", dn );
            // rebind
            ldap_initialize(&ld2, LDAP_SERVER);

            //int protocol_version = LDAP_VERSION3;
            rc = ldap_set_option(ld2, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
            if (rc != LDAP_SUCCESS) {
                fprintf(stderr, "ldap_simple_bind_s: %s\n", ldap_err2string(rc));
                return(1);
            }


            const char * password2 = "Anyks6991163";
            
            #if defined(LDAP_API_FEATURE_X_OPENLDAP) && LDAP_API_FEATURE_X_OPENLDAP >= 20300
              struct berval cred2 = { 0, NULL };
              cred2.bv_len = strlen(password2);
              cred2.bv_val = new char[cred2.bv_len + 1]; //malloc(cred.bv_len+1);
              strcpy(cred2.bv_val, password2);
              rc = ldap_sasl_bind_s (ld2, dn, LDAP_SASL_SIMPLE, &cred2, NULL, NULL, NULL);
              delete [] cred2.bv_val;
             // free(cred.bv_val);
              memset(&cred2, 0, sizeof(cred2));
            #else
              rc = ldap_simple_bind_s (ld2, dn, password2);
            #endif
            

            /*
            struct berval cred = { 0, NULL };
            cred.bv_len = strlen(password);
            cred.bv_val = new char[cred.bv_len + 1]; //malloc(cred.bv_len+1);
            strcpy(cred.bv_val, password);
            
            rc = ldap_compare_ext_s(ld2, dn, "userPassword", &cred, NULL, NULL);
            delete [] cred.bv_val;
            memset(&cred, 0, sizeof(cred));
            */

            //rc = ldap_simple_bind_s(ld2, dn, "Anyks6991163");
            printf("%d\n", rc);
            if (rc != 0) {
                printf("Failed.\n");
            } else {
                printf("Works.\n");
                ldap_unbind_ext(ld2, NULL, NULL);
            }
            ldap_memfree( dn );
        }
        
        /*
        if ( (dn = ldap_get_dn( ld, e )) != NULL ) {
            printf( "dn: %s\n", dn );
            has_value = ldap_compare_s( ld, dn, "userPassword", "Anyks6991163" ); 
            switch ( has_value ) { 
                case LDAP_COMPARE_TRUE: 
                    printf( "Works.\n"); 
                    break; 
                case LDAP_COMPARE_FALSE: 
                    printf( "Failed.\n"); 
                    break; 
                default: 
                    ldap_perror( ld, "ldap_compare_s" ); 
                    return( 1 ); 
            } 
            ldap_memfree( dn );
        }
        */
    }

    ldap_msgfree( result );
    ldap_unbind_ext(ld, NULL, NULL);
    return( 0 );
}
