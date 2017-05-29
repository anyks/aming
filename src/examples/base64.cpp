// $ g++ -std=c++11 -Wall -Wpedantic -Wstrict-aliasing -Werror=vla -Wno-unused-result -g -std=c++11 -O2 -lz -pipe -fomit-frame-pointer -fstrict-aliasing -mavx2 -mbmi2 /usr/local/Cellar/openssl@1.1/1.1.0e/lib/libcrypto.a /usr/local/Cellar/openssl@1.1/1.1.0e/lib/libssl.a -I/usr/local/Cellar/openssl@1.1/1.1.0e/include ./examples/base64.cpp -o ./bin/base64
// $ clang++ -std=c++11 -Wall -Wpedantic -Wstrict-aliasing -Werror=vla -Wno-unused-result -g -std=c++11 -O2 -ggdb -lz -pipe -fomit-frame-pointer -fstrict-aliasing -mavx2 -mbmi2 -lcrypto -I/usr/local/include ./examples/base64.cpp -o ./bin/base64
// $ ./bin/base64

#include <stdio.h>
#include <string>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <stdint.h>

#define OP_ENCODE 0
#define OP_DECODE 1
int b64_op(const unsigned char* in, int in_len,
              char *out, int out_len, int op)
{
    int ret = 0;
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);
    if (op == 0)
    {
        ret = BIO_write(b64, in, in_len);
        BIO_flush(b64);
        if (ret > 0)
        {
            ret = BIO_read(bio, out, out_len);
        }

    } else
    {
        ret = BIO_write(bio, in, in_len);
        BIO_flush(bio);
        if (ret)
        {
            ret = BIO_read(b64, out, out_len);
        }
    }
    BIO_free(b64);
    return ret;
}

int main(void)
{
    const string enc_data = "grrr shebangit!";
    char out[256];
    char orig[256];

    int enc_out_len = b64_op(enc_data.c_str(), enc_data.length(), out, sizeof(out), OP_ENCODE);

    printf("Enc data [%s] len [%d]\n",
           out, enc_out_len);

    int dec_out_len =
            b64_op((const unsigned char*) out, enc_out_len,
                  orig, sizeof(orig), OP_DECODE);

    printf("Dec data [%s] len [%d]\n",
           orig, dec_out_len);

    return 0;
}