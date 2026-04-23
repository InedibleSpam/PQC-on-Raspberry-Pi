#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/rsa.h>

/**
 * get_time:
 * Uses CLOCK_MONOTONIC to get the current time with nanosecond precision.
 * This is essential for measuring sub-millisecond operations like ECDSA verify.
 */
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

/**
 * print_comm_size:
 * Helper function to output the 'Communication Size' data.
 * This provides the raw numbers needed to evaluate 'Network Viability' (MTU limits).
 */
void print_comm_size(const char* alg, size_t pk_size, size_t payload_size) {
    printf("%s Comm Size: PK=%zu, Payload=%zu, Total=%zu bytes\n", 
            alg, pk_size, payload_size, pk_size + payload_size);
}

int main() {
    printf("Running Classical Crypto Comparison (ECDH, ECDSA, RSA)\n");
    double start, end;
    unsigned char message[] = "Performance test for ECDSA and RSA signatures";
    size_t msg_len = strlen((char *)message);

    /* --- 1. ECDH SECTION (Elliptic Curve Diffie-Hellman) --- 
       Baseline for PQC Key Encapsulation (KEM) comparisons. */
    
    // Initialize context for P-256 curve
    EVP_PKEY_CTX *pctx_ec = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(pctx_ec);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx_ec, NID_X9_62_prime256v1);
    
    EVP_PKEY *ec_key1 = NULL;
    EVP_PKEY *ec_key2 = NULL;
    
    // Measure Key Generation time (The 'Client' side of the handshake)
    start = get_time();
    EVP_PKEY_keygen(pctx_ec, &ec_key1);
    end = get_time();
    
    // Retrieve the Public Key length to calculate Communication Size
    size_t ec_pk_len = 0;
    EVP_PKEY_get_raw_public_key(ec_key1, NULL, &ec_pk_len);
    printf("ECDH KeyGen time: %f seconds\n", end - start);

    // Generate a second key to simulate a peer
    EVP_PKEY_keygen(pctx_ec, &ec_key2);
    EVP_PKEY_CTX *derive_ctx = EVP_PKEY_CTX_new(ec_key1, NULL);
    EVP_PKEY_derive_init(derive_ctx);
    EVP_PKEY_derive_set_peer(derive_ctx, ec_key2);
    
    // Measure Derivation time (The 'Server' side shared secret calculation)
    size_t secret_len;
    start = get_time();
    EVP_PKEY_derive(derive_ctx, NULL, &secret_len);
    unsigned char *secret = malloc(secret_len);
    EVP_PKEY_derive(derive_ctx, secret, &secret_len);
    end = get_time();
    printf("ECDH Derivation time: %f seconds\n", end - start);
    
    // Output network footprint: Public Key + Derived Shared Secret
    print_comm_size("ECDH", ec_pk_len, secret_len);

    /* --- 2. ECDSA SECTION (Signatures) --- 
       Baseline for PQC Signature (ML-DSA/Falcon) comparisons. */
    
    unsigned char ec_sig[256];
    size_t ec_sig_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    
    // Measure Signing time
    start = get_time();
    EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, ec_key1);
    EVP_DigestSign(mdctx, ec_sig, &ec_sig_len, message, msg_len);
    end = get_time();
    printf("ECDSA Sign time: %f seconds\n", end - start);

    // Measure Verification time
    start = get_time();
    EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, ec_key1);
    EVP_DigestVerify(mdctx, ec_sig, ec_sig_len, message, msg_len);
    end = get_time();
    printf("ECDSA Verify time: %f seconds\n", end - start);
    
    // Output network footprint: Public Key + Signature length
    print_comm_size("ECDSA", ec_pk_len, ec_sig_len);

    /* --- 3. RSA SECTION (RSA-3072) --- 
       High-security classical baseline (roughly equivalent to PQC Level 1). */
    
    EVP_PKEY_CTX *pctx_rsa = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(pctx_rsa);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx_rsa, 3072);

    EVP_PKEY *rsa_key = NULL;
    // RSA KeyGen is notoriously slow on Pi; good for showing PQC efficiency
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
    
    // Calculate RSA Public Key size (usually 384 bytes for 3072-bit keys)
    size_t rsa_pk_len = EVP_PKEY_size(rsa_key);
    print_comm_size("RSA", rsa_pk_len, rsa_sig_len);

    /* --- CLEANUP --- */
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