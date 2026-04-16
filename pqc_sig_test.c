#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h> // Using the Open Quantum Safe library

// Standard helper to measure time in seconds
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main() {
    printf("Running PQC signature test\n");
    
    // --- INITIALIZATION ---
    // Create a signature object using "Falcon-512"
    // Falcon is known for having very small signatures and being very fast to verify.
    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_falcon_512);

    if (sig == NULL) {
        printf("Signature init failed");
        return 1;
    }
    
    // --- MEMORY ALLOCATION ---
    // Unlike traditional crypto, PQC keys and signatures can be quite large.
    // We allocate exactly what the Falcon algorithm asks for.
    uint8_t *public_key = malloc(sig->length_public_key);
    uint8_t *secret_key = malloc(sig->length_secret_key);
    uint8_t *signature = malloc(sig->length_signature);

    // The data we want to sign
    uint8_t message[] = "Hello from PQC";
    size_t message_len = strlen((char *)message);

    size_t signature_len; // To store the actual length of the resulting signature
    double start, end;

    // --- KEY GENERATION ---
    // Creates a Public Key (to share) and a Secret Key (to keep for signing)
    start = get_time();
    sig->keypair(public_key, secret_key);
    end = get_time();
    printf("KeyGen time: %f seconds\n", end-start);
    

    // --- SIGNING ---
    // Use the secret_key to create a mathematical proof that "Hello from PQC"
    // was approved by the owner of this key.
    start = get_time();
    sig->sign(signature, &signature_len, message, message_len, secret_key);
    end = get_time();
    printf("Sign time:%f seconds\n", end-start);

    // --- VERIFICATION ---
    // Anyone with the public_key can check if the signature is valid.
    // Result will be OQS_SUCCESS if the message hasn't been changed.
    start = get_time();
    int result = sig->verify(message, message_len, signature, signature_len, public_key);
    end = get_time();
    printf("Verify time: %f seconds\n", end-start);

    // Check if the math worked out
    if (result == OQS_SUCCESS) {
        printf("Signature verified successfully\n");
    } else {
        printf("Signature verification failed\n");
    }

    // --- CLEANUP ---
    // Free the buffers and the algorithm object to prevent memory leaks
    free(public_key);
    free(secret_key);
    free(signature);
    OQS_SIG_free(sig);

    return 0;
}