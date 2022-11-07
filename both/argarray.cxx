#include "argarray.hxx"

std::vector<std::string> split_command(char strToSplit[])
{
    char* strSplit = strToSplit;
    char* token;
    std::string total;
    std::vector<std::string> array;

    // loop full string until there are no spaces left in the string
    while (token = strtok_s(strSplit, " ", &strSplit)) 
    {
        if (token[0] == '\'' || token[0] == '"') //start quote
        {
            // first add this to the string total. i am using total as a conatiner
            total.append(token);

            while (!(token && *token && token[strlen(token) - 1] == '\''))
            {
                // keep looping until the token has an end quote
                token = strtok_s(strSplit, " ", &strSplit);
                total.append(" "); total.append(token);
            }
            // add results to the next index in the array
            array.push_back(total.c_str());
        }
        else
        {   // if no quotes -> add as is
            array.push_back(token);
        }
    }
    return array;
}
