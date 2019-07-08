

#include <bls/bls.h>
#include <dkg/dkg.h>
#include <ctime>

#include <map>

#define BOOST_TEST_MODULE

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Bls)

BOOST_AUTO_TEST_CASE(libBls){


        std::cerr << "STARTING LIBBLS TESTS";
        for (size_t i = 0; i < 100; ++i) {


            std::srand(unsigned(std::time(0)));
            size_t num_all = std::rand()%16 + 1;
            size_t num_signed = std::rand()%num_all + 1;
            nums.push_back(std::make_pair(num_signed,num_all));
            signatures::Dkg dkg_obj = signatures::Dkg( num_signed, num_all);
            const std::vector<libff::alt_bn128_Fr> pol = dkg_obj.GeneratePolynomial();
            std::vector<libff::alt_bn128_Fr> skeys = dkg_obj.SecretKeyContribution(pol);

            std::vector<libff::alt_bn128_G1> signatures(num_signed);

            signatures::Bls obj = signatures::Bls(num_signed, num_all);

            for (size_t i = 0; i < 10; ++i) {
                std::string message = "";
                size_t msg_length = std::rand() % 1000 + 2;
                for (size_t length = 0; length < msg_length; ++length) {
                    message += char(std::rand() % 128);
                }

                libff::alt_bn128_G1 hash = obj.Hashing(message);
                for (size_t i = 0; i < num_signed; ++i) signatures[i] = obj.Signing(hash, skeys[i]);

                //std::vector<size_t> participants(num_signed);
               // for (size_t i = 0; i < num_signed; ++i) participants[i] = i + 1;

                std::vector<size_t> participants(num_all);
                for (size_t i = 0; i < num_all; ++i) participants[i] = i + 1;
                for (size_t i = 0; i < num_all - num_signed; ++i){
                    size_t ind4del = std::rand()%participants.size();
                    participants.erase(participants.begin() + ind4del);
                }

                for (size_t i = 0; i < num_signed; ++i){
                    auto pkey = skeys[i] *  libff::alt_bn128_G2::one();
                    BOOST_REQUIRE( obj.Verification(message, signatures[i], pkey) );
                }

                    std::vector<libff::alt_bn128_Fr> lagrange_coeffs = obj.LagrangeCoeffs(participants);
                libff::alt_bn128_G1 signature = obj.SignatureRecover(signatures, lagrange_coeffs);

                auto recovered_keys = obj.KeysRecover(lagrange_coeffs, skeys);
                BOOST_REQUIRE(obj.Verification(message, signature, recovered_keys.second));

                bool is_exception_caught = false;
                try{
                    size_t n_bad_bit = std::rand() % (signature.X.size_in_bits()) + 1;

                    mpz_t was_X;
                    mpz_init(was_X);
                    signature.X.as_bigint().to_mpz(was_X);

                    mpz_t mask;
                    mpz_init(mask);
                    mpz_set_si(mask, n_bad_bit);

                    mpz_t badX;
                    mpz_init(badX);
                    mpz_xor(badX, was_X, mask);

                    signature.X = libff::alt_bn128_Fq(badX);

                    obj.Verification(message, signature, recovered_keys.second);
                    mpz_clears(badX, was_X, mask);
                }
                catch (std::runtime_error){
                    is_exception_caught = true;
                }

                BOOST_REQUIRE(is_exception_caught);

            }

        }

        std::cerr << "BLS TESTS completed successfully";


}

BOOST_AUTO_TEST_SUITE_END()
