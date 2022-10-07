#include <iostream>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <vector>
#include <climits>

#include "../src/ChunkedEncoding.h"

void runTest(unsigned int maxConsume) {
    /* A list of chunks */
    std::vector<std::string_view> chunks = {
        "Hello there I am the first segment",
        "Why hello there",
        "",
        "I am last?",
        "And I am a little longer but it doesn't matter",
        ""
    };

    /* Encode them in chunked encoding */
    std::stringstream ss;
    for (std::string_view chunk : chunks) {

        // if length is 0 then append trailer also

        ss << std::hex << chunk.length() << "\r\n" << chunk << "\r\n";

        // every null chunk is followed by an empty trailer
        if (chunk.length() == 0) {
            //ss << "\r\n";
        }
    }

    /* Consume them, checking that we get what we expect */
    std::string buffer = ss.str();
    unsigned int state = 0;
    std::string_view chunkEncoded = buffer;

    unsigned int consumed = UINT_MAX;
    int chunkOffset = 0;
    while (consumed) {
        /* Consume up to maxConsume */
        std::string_view indata = chunkEncoded.substr(0, std::min<size_t>(maxConsume, chunkEncoded.length())); 

        consumed = uWS::consumeChunkedEncoding(indata, state, [&](std::string_view chunk) {
            /* Print for logging */  
            std::cout << "<" << chunk << ">";

            if (!chunk.length() && chunks[chunkOffset].length()) {
                std::cout << "We got emitted an empty chunk but expected a non-empty one" << std::endl;
                std::abort();
            }

            if (chunks[chunkOffset].starts_with(chunk)) {
                chunks[chunkOffset].remove_prefix(chunk.length());
                if (!chunks[chunkOffset].length()) {
                    chunkOffset++;
                }
            } else {
                std::cerr << "Chunk does not match! Should be <" << chunks[chunkOffset] << ">" << std::endl;
                std::abort();                
            }
        });
        if (consumed != indata.length()) {
            std::cerr << "Chunk parser did not consume exactly the bytes passed!" << std::endl;
            std::abort();
        }
        chunkEncoded.remove_prefix(consumed);
    }
}

int main() {
    for (int i = 0; i < 1000; i++) {
        runTest(i);
    }

    std::cout << "ALL BRUTEFORCE DONE" << std::endl;
}