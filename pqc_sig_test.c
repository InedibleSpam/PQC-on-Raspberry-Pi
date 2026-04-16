#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

// Standard helper to measure high-precision time in seconds
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main() {
    printf("Running PQC Signature Comparison Test\n");

    // Updated array using the constants your specific library version expects
    const char *algorithms[] = {
        OQS_SIG_alg_ml_dsa_44,            // NIST Level 2 (Dilithium)
        OQS_SIG_alg_falcon_512,           // NIST Level 1 (Falcon)
        OQS_SIG_alg_slh_dsa_pure_sha2_128f // NIST Level 1 (SPHINCS+)
    };
    int num_algs = sizeof(algorithms) / sizeof(algorithms[0]);

    for (int i = 0; i < num_algs; i++) {
        printf("\n--- Algorithm: %s ---\n", algorithms[i]);

        // --- INITIALIZATION ---
        // Load the signature algorithm object
        OQS_SIG *sig = OQS_SIG_new(algorithms[i]);
        if (sig == NULL) {
            printf("Initialization failed for %s\n", algorithms[i]);
            continue;
        }
        
        // --- MEMORY ALLOCATION ---
        // PQC signatures and keys vary wildly in size.
        // Falcon is small, ML-DSA is medium, SLH-DSA keys are tiny but signatures are huge.
        uint8_t *public_key = malloc(sig->length_public_key);
        uint8_t *secret_key = malloc(sig->length_secret_key);
        uint8_t *signature = malloc(sig->length_signature);

        uint8_t message[] = "Performance testing for NIST PQC Standards";
        size_t message_len = strlen((char *)message);
        size_t signature_len;
        double start, end;

        // --- KEY GENERATION ---
        // Create the Public and Secret key pair
        start = get_time();
        sig->keypair(public_key, secret_key);
        end = get_time();
        printf("%s KeyGen time: %f seconds\n", algorithms[i], end - start);

        // --- SIGNING ---
        // Prove the message is authentic using the secret key
        start = get_time();
        sig->sign(signature, &signature_len, message, message_len, secret_key);
        end = get_time();
        printf("%s Sign time: %f seconds\n", algorithms[i], end - start);

        // --- VERIFICATION ---
        // Check the signature using the public key
        start = get_time();
        int result = sig->verify(message, message_len, signature, signature_len, public_key);
        end = get_time();
        printf("%s Verify time: %f seconds\n", algorithms[i], end - start);

        if (result == OQS_SUCCESS) {
            printf("Result: Signature verified!\n");
        } else {
            printf("Result: ERROR - Verification failed!\n");
        }

        // --- CLEANUP ---
        // Always free buffers and the algorithm object
        free(public_key);
        free(secret_key);
        free(signature);
        OQS_SIG_free(sig);
    }

    return 0;
}