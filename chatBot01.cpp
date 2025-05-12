// dH 11/20/24
// chatBot01.cpp
// Hey Professor, Just wanted to say thanks for the class, it was fantastic.
//I apologize for using your code modified, but I've got a health crisis situation
//Everything is pointing to a good outcome, but it's still a very rough time right now.
//And I just didn't/couldn't find the motivation to do everything from scratch.
//Instead I opted to enjoy time with friends and call family.
//I modified the existing code and it should do everything that is required for the assignment.
//Thanks again.
//EM 5/11/2025

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>

using json = nlohmann::json;
using namespace std;

// Callback function for cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* out) {
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

// Function to fetch the current time in Italy from WorldTimeAPI
string getTimeInItaly() {
    string responseString;
    CURL* curl = curl_easy_init();

    if (curl) {
        string url = "https://worldtimeapi.org/api/timezone/Europe/Rome";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Use a CA certificate bundle for SSL verification
        curl_easy_setopt(curl, CURLOPT_CAINFO, "C:/2024_Fall/cit66_Cpp/chatBot01/cacert.pem");

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "cURL error: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
    }

    if (responseString.empty()) {
        return "Error: Could not fetch the time. Please try again later.";
    }

    // Parse the JSON response
    try {
        json jsonResponse = json::parse(responseString);
        if (jsonResponse.contains("datetime")) {
            return jsonResponse["datetime"];
        } else {
            return "Error: Unexpected response format from WorldTimeAPI.";
        }
    } catch (const json::exception& e) {
        return "Error: Failed to parse response from WorldTimeAPI.";
    }
}

// Function to send a message to OpenAI API
string sendMessageToChatbot(const string& userMessage, const string& apiKey, int API_Call_Tries) {
    if (API_Call_Tries == 4){
        cerr << "Error: Empty response from API exceeded 3 attempts." << endl;
        return "";
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    string responseString;
    CURL* curl = curl_easy_init();
    if (curl) {
        string url = "https://api.openai.com/v1/chat/completions";
        string payload = R"({
            "model": "gpt-3.5-turbo",
            "messages": [{"role": "user", "content": ")" + userMessage + R"("}]
        })";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Use a CA certificate bundle for SSL verification
        curl_easy_setopt(curl, CURLOPT_CAINFO, "C:/2024_Fall/cit66_Cpp/chatBot01/cacert.pem");

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "cURL error: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if (responseString.empty()) {
        API_Call_Tries++;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return sendMessageToChatbot(userMessage, apiKey, API_Call_Tries);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time).count();
    cout << "This API call took a total of " << duration << " ms.\n";
    return responseString;
}

int main() {
    int chat_inter_count = 0;
    ostringstream stream;
    vector<pair<string, string>> Chat_history;
    fstream myFile;
    string userMessage;
    string chatbotName = "Assistant";  // Default chatbot name
    string userName = "User";          // Default user name

    //Not hardcoding API key, Reading from Text file and placing in variable for security purposes.
    string personal_key;
    myFile.open("C:/Users/eckst/Desktop/Key.txt", ios::in);
    if (myFile.is_open()){
      string line_from_file;
      while(getline(myFile,line_from_file)){
        personal_key = line_from_file;
      }
    };
    cout << "Chatbot (type 'exit' to quit):\n";
    while (true) {
        cout << "> ";
        getline(cin, userMessage);

        if (userMessage == "exit") break;

        // Handle "name assignment" logic
        stream.str(""); stream.clear();

        if (userMessage.empty() || userMessage.length() > 350) {
            while (true) {
                cout << "User input is empty or you used too many characters. Please try again.\n";
                cout << "> ";
                getline(cin, userMessage);

                if (!userMessage.empty() && userMessage.length() <= 500) {
                    break;
                }
            }
        }
        if (userMessage.find("Your name is now") != string::npos) {
            size_t start = userMessage.find("Your name is now") + 17;
            chatbotName = userMessage.substr(start);
            chatbotName.erase(chatbotName.find_last_not_of(" \n\r\t") + 1);  // Trim whitespace
            cout << "Bot: Okay, I will now call myself " << chatbotName << ". What else can I do for you?" << endl;
            stream << "Bot: Okay, I will now call myself " << chatbotName << ". What else can I do for you?" << endl;
            string bot_response = stream.str();
            Chat_history.push_back({userMessage, bot_response});
            chat_inter_count++;
            continue;
        }
        if (userMessage.find("my name is") != string::npos) {
            size_t start = userMessage.find("my name is") + 10;
            userName = userMessage.substr(start);
            userName.erase(userName.find_last_not_of(" \n\r\t") + 1);  // Trim whitespace
            cout << chatbotName << ": Nice to meet you, " << userName << "! How can I assist you today?" << endl;
            stream << chatbotName << ": Nice to meet you, " << userName << "! How can I assist you today?" << endl;
            string bot_response = stream.str();
            Chat_history.push_back({userMessage, bot_response});
            chat_inter_count++;
            continue;
        }

        // Handle "time in Italy" queries directly
        if (userMessage.find("time in Italy") != string::npos) {
            string timeInItaly = getTimeInItaly();
            cout << chatbotName << ": The current time in Italy is: " << timeInItaly << "\n";
            stream << chatbotName << ": The current time in Italy is: " << timeInItaly << "\n";
            string bot_response = stream.str();
            Chat_history.push_back({userMessage, bot_response});
            chat_inter_count++;
            continue;
        }

        // Send other queries to OpenAI API
        int API_Call_Tries = 0;
        string response = sendMessageToChatbot(userMessage, personal_key, API_Call_Tries);

        try {
            json jsonResponse = json::parse(response);

            // Check if the response has the expected structure
            if (jsonResponse.contains("choices") && !jsonResponse["choices"].empty()) {
                if (jsonResponse["choices"][0]["message"].contains("content") && !jsonResponse["choices"][0]["message"]["content"].is_null()) {
                    string chatbotReply = jsonResponse["choices"][0]["message"]["content"];
                    cout << chatbotName << ": " << chatbotReply << "\n";
                    stream << chatbotName << ": " << chatbotReply << "\n";
                    string bot_response = stream.str();
                    Chat_history.push_back({userMessage, bot_response});
                    chat_inter_count++;
                } else {
                    cerr << chatbotName << ": Sorry, I didn't understand the response. Please try again." << endl;
                    stream << chatbotName << ": Sorry, I didn't understand the response. Please try again." << endl;
                    string bot_response = stream.str();
                    Chat_history.push_back({userMessage, bot_response});
                    chat_inter_count++;
                }
            } else {
                cerr << chatbotName << ": Sorry, I couldn't get a valid response from the server." << endl;
                stream << chatbotName << ": Sorry, I couldn't get a valid response from the server." << endl;
                string bot_response = stream.str();
                Chat_history.push_back({userMessage, bot_response});
                chat_inter_count++;
            }
        } catch (const json::exception& e) {
            cerr << chatbotName << ": JSON parsing error: " << e.what() << "\n";
            stream << chatbotName << ": JSON parsing error: " << e.what() << "\n";
            string bot_response = stream.str();
            Chat_history.push_back({userMessage, bot_response});
            chat_inter_count++;
        }
        cout << "Chat History Printout: " << endl;
        for (const auto& pair : Chat_history){
          cout << pair.first << "\n" << pair.second << "\n";
        }
    }

    return 0;
}
