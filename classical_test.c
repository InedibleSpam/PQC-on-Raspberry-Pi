#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/rsa.h> // CRITICAL: This fixes the 'undeclared' warning

// Standard helper to measure high-precision time in seconds
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(){
    printf("Running Classical Crypto Comparison (ECDH, ECDSA, RSA)\n");
    double start, end;
    unsigned char message[] = "Performance test for ECDSA and RSA signatures";
    size_t msg_len = strlen((char *)message);

    // --- 1. ECDH SECTION (Key Exchange) ---
    EVP_PKEY_CTX *pctx_ec = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(pctx_ec);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx_ec, NID_X9_62_prime256v1);
    
    EVP_PKEY *ec_key1 = NULL;
    EVP_PKEY *ec_key2 = NULL;
    
    // Generate Key 1
    start = get_time();
    EVP_PKEY_keygen(pctx_ec, &ec_key1);
    end = get_time();
    printf("ECDH KeyGen time: %f seconds\n", end - start);

    // Generate Key 2 (needed for derivation)
    EVP_PKEY_keygen(pctx_ec, &ec_key2);

    // Derivation (The actual "Exchange" part)
    EVP_PKEY_CTX *derive_ctx = EVP_PKEY_CTX_new(ec_key1, NULL);
    EVP_PKEY_derive_init(derive_ctx);
    EVP_PKEY_derive_set_peer(derive_ctx, ec_key2);
    
    size_t secret_len;
    start = get_time();
    EVP_PKEY_derive(derive_ctx, NULL, &secret_len);
    unsigned char *secret = malloc(secret_len);
    EVP_PKEY_derive(derive_ctx, secret, &secret_len);
    end = get_time();
    printf("ECDH Derivation time: %f seconds\n", end - start);

    // --- 2. ECDSA SECTION (Signatures) ---
    unsigned char ec_sig[256];
    size_t ec_sig_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    
    start = get_time();
    EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, ec_key1);
    EVP_DigestSign(mdctx, ec_sig, &ec_sig_len, message, msg_len);
    end = get_time();
    printf("ECDSA Sign time: %f seconds\n", end - start);

    start = get_time();
    EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, ec_key1);
    EVP_DigestVerify(mdctx, ec_sig, ec_sig_len, message, msg_len);
    end = get_time();
    printf("ECDSA Verify time: %f seconds\n", end - start);

    // --- 3. RSA SECTION (RSA-3072) ---
    EVP_PKEY_CTX *pctx_rsa = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(pctx_rsa);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx_rsa, 3072);

    EVP_PKEY *rsa_key = NULL;
    start = get_time();
    EVP_PKEY_keygen(pctx_rsa, &rsa_key);
    end = get_time();
    printf("RSA-3072 KeyGen time: %f seconds\n", end - start);

    unsigned char rsa_sig[512];
    size_t rsa_sig_len;

    start = get_time();
    EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, rsa_key);
    EVP_DigestSign(mdctx, rsa_sig, &rsa_sig_len, message, msg_len);
    end = get_time();
    printf("RSA-3072 Sign time: %f seconds\n", end - start);

    start = get_time();
    EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, rsa_key);
    EVP_DigestVerify(mdctx, rsa_sig, rsa_sig_len, message, msg_len);
    end = get_time();
    printf("RSA-3072 Verify time: %f seconds\n", end - start);

    // --- CLEANUP ---
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(ec_key1);
    EVP_PKEY_free(ec_key2);
    EVP_PKEY_free(rsa_key);
    EVP_PKEY_CTX_free(pctx_ec);
    EVP_PKEY_CTX_free(pctx_rsa);
    EVP_PKEY_CTX_free(derive_ctx);
    free(secret);

    return 0;
}