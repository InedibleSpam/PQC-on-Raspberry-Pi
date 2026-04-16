#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

// Helper function to measure time in seconds with high precision
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(){
    printf("Running Classical Crypto Test\n\n");
    
    double start, end;

    // Pointers for our keys and the context (settings) used to create them
    EVP_PKEY_CTX *pctx;
    EVP_PKEY *key1 = NULL;
    EVP_PKEY *key2 = NULL;

    // --- KEY GENERATION SETUP ---
    // Create a context for Elliptic Curve (EC) key generation
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(pctx);

    // Set which curve to use (NID_X9_62_prime256v1 is the "P-256" standard curve)
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);
    
    // Generate the first key and time it
    start = get_time();
    EVP_PKEY_keygen(pctx, &key1);
    end = get_time();
    printf("ECDH KeyGen time: %f seconds\n", end - start);

    // Generate a second key (simulating a second person in the conversation)
    EVP_PKEY_keygen(pctx, &key2);

    // --- SHARED SECRET DERIVATION (ECDH) ---
    EVP_PKEY_CTX *derive_ctx;
    size_t secret_len;
    unsigned char *secret;
    
    // Create a context for deriving a secret using key1
    derive_ctx = EVP_PKEY_CTX_new(key1, NULL);
    EVP_PKEY_derive_init(derive_ctx);

    // Provide the other person's public key (key2) to combine with our private key
    EVP_PKEY_derive_set_peer(derive_ctx, key2);

    start = get_time();
    // First call: find out how many bytes the resulting secret will be
    EVP_PKEY_derive(derive_ctx, NULL, &secret_len);
    // Allocate memory for that secret
    secret = malloc(secret_len);
    // Second call: perform the actual math to create the shared secret
    EVP_PKEY_derive(derive_ctx, secret, &secret_len);
    end = get_time();

    printf("ECDH Shared Secret Time: %f seconds\n", end - start);

    // --- SIGNING AND VERIFICATION (ECDSA) ---
    // This is the message we want to sign (a poem by John Masefield)
    unsigned char message[] = "We were schooner-rigged and rakish..."; // (truncated for brevity)
    size_t msg_len = strlen((char *)message);
    
    unsigned char signature[256];
    size_t sig_len;

    // Create a Message Digest (MD) context for signing/verifying
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

    // SIGNING: Prove the message came from the owner of key1
    start = get_time();
    // Initialize the signature process using SHA-256 as the hashing algorithm
    EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, key1);
    // Create the digital signature
    EVP_DigestSign(mdctx, signature, &sig_len, message, msg_len);
    end = get_time();

    printf("ECDSA Sign time: %f seconds\n", end - start);

    // VERIFICATION: Check if the signature is valid for this message and key
    start = get_time();
    EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, key1);
    EVP_DigestVerify(mdctx, signature, sig_len, message, msg_len);
    end = get_time();

    printf("ECDSA Verify time; %f seconds\n" , end - start);

    // --- CLEANUP ---
    // Always free memory in C to prevent leaks
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(key1);
    EVP_PKEY_free(key2);
    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_CTX_free(derive_ctx);
    free(secret);

    return 0;
}