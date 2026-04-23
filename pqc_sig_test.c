#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

/**
 * get_time:
 * Essential for capturing the 'Execution Latency' baseline.
 * Monotonic clock ensures measurements aren't affected by system time updates.
 */
double get_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

/**
 * print_comm_size:
 * Outputs the 'Communication Size' (Public Key + Signature).
 * This is used to prove if an algorithm fits in a 1,500-byte MTU packet.
 */
void print_comm_size(const char* alg, size_t pk_size, size_t sig_size) {
    printf("%s Comm Size: PK=%zu, Sig=%zu, Total=%zu bytes\n", 
            alg, pk_size, sig_size, pk_size + sig_size);
}

int main() {
    printf("Running PQC Signature Comparison Test\n");

    const char *algorithms[] = {
        OQS_SIG_alg_ml_dsa_44,              // Dilithium (NIST Winner)
        OQS_SIG_alg_falcon_512,             // Falcon (Fastest Verify)
        OQS_SIG_alg_slh_dsa_pure_sha2_128f  // SPHINCS+ (Stateless/Large Sig)
    };
    int num_algs = sizeof(algorithms) / sizeof(algorithms[0]);

    for (int i = 0; i < num_algs; i++) {
        printf("\n--- Algorithm: %s ---\n", algorithms[i]);

        // --- INITIALIZATION ---
        OQS_SIG *sig = OQS_SIG_new(algorithms[i]);
        if (sig == NULL) {
            printf("Initialization failed for %s\n", algorithms[i]);
            continue;
        }
        
        // --- MEMORY ALLOCATION ---
        // Allocation sizes are pulled directly from the algorithm's metadata
        uint8_t *public_key = malloc(sig->length_public_key);
        uint8_t *secret_key = malloc(sig->length_secret_key);
        uint8_t *signature = malloc(sig->length_signature);

        uint8_t message[] = "Performance testing for NIST PQC Standards";
        size_t message_len = strlen((char *)message);
        size_t signature_len;
        double start, end;

        // --- KEY GENERATION ---
        start = get_time();
        sig->keypair(public_key, secret_key);
        end = get_time();
        printf("%s KeyGen time: %f seconds\n", algorithms[i], end - start);

        // --- SIGNING ---
        // Uses the secret key to generate a signature
        start = get_time();
        sig->sign(signature, &signature_len, message, message_len, secret_key);
        end = get_time();
        printf("%s Sign time: %f seconds\n", algorithms[i], end - start);

        // --- VERIFICATION ---
        // Uses the public key to check authenticity
        start = get_time();
        int result = sig->verify(message, message_len, signature, signature_len, public_key);
        end = get_time();
        printf("%s Verify time: %f seconds\n", algorithms[i], end - start);

        // Report the 'Solid Numbers' for the communication footprint
        print_comm_size(algorithms[i], sig->length_public_key, signature_len);

        // --- CLEANUP ---
        free(public_key);
        free(secret_key);
        free(signature);
        OQS_SIG_free(sig);
    }
    return 0;
}