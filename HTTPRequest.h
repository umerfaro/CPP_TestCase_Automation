#pragma once
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
using namespace std;
 

class HttpRequest 
{
public:

    string promtInfo = "";

    HttpRequest(const std::string& apiKey, const std::string& apiUrl)
        : apiKey(apiKey), apiUrl(apiUrl), responseBuffer(nullptr) {
        // Initialize libcurl
        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        }
    }

    ~HttpRequest() {
        // Clean up libcurl
        if (curl) {
            curl_easy_cleanup(curl);
        }
        // Clean up response buffer
        if (responseBuffer) {
            delete[] responseBuffer;
        }
    }

    // Function to make an HTTP POST request
    void post(const std::string& data) {
        if (!curl) {
            std::cerr << "Error: libcurl is not initialized." << std::endl;
            return;
        }

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

        // Set the HTTP method to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        // Set headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set callback to write the response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up headers
        curl_slist_free_all(headers);
    }

    // Function to get the response data
    const std::string& getResponse() const {
        return response;
    }


    void extractContent(const std::string& jsonResponse ) 
    {
        try {
            // Parse JSON
            nlohmann::json responseJson = nlohmann::json::parse(jsonResponse);

            // Check if the "choices" array exists
            if (responseJson.contains("choices") && responseJson["choices"].is_array()) {
                // Extract content from the first choice
                std::string content = responseJson["choices"][0]["message"]["content"];

                // Display or store the content as needed
                std::cout << "Generated response: " << content << std::endl;
                promtInfo += content;
            }
            else {
                std::cerr << "Error: Unable to extract content from the response." << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: Failed to parse JSON response. " << e.what() << std::endl;
        }
    }




private:
    CURL* curl;
    std::string apiKey;
    std::string apiUrl;
    std::string response;
    char* responseBuffer;

    // Callback function to write response data
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t realsize = size * nmemb;
        HttpRequest* self = static_cast<HttpRequest*>(userp);

        // Allocate memory for the response buffer
        self->responseBuffer = static_cast<char*>(realloc(self->responseBuffer, self->response.size() + realsize + 1));

        if (self->responseBuffer) {
            memcpy(&(self->responseBuffer[self->response.size()]), contents, realsize);
            self->responseBuffer[realsize] = 0;

            // Append the received data to the response string
            self->response += self->responseBuffer;
            return realsize;
        }

        return 0;
    }
};


