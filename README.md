Programming Assignment 1

Francisco Gomez

__httpserver.c__:
This is the main file. Where everything is ran and to which the header files are attached to. It is composed of multiple helper functions and one large main function body.  
- parse Request is hardly used and is only used to extract the length size of the information in the request such as the URI or the METHOD.  
- __PARSER__: Used to parse the information from the request into buffers. Store said information into a buffer via pass by reference.  
- __sendresponse__: send response sends back a message to the client with a message body and a content length. Depending if it is PUT or an error message, those exceptions are the only ones where a message body will be included.  
- __illChecker__: Parses the header field and checks for ill formated request. 

- __MAIN__: This is the heart of the operation.  
-Creates a listening socket and accepts incoming requests.  
-in a while loops, reads the header field into a buffer of size 2048  
-it calls illchecker for ill formated requests.  
-checks the size of the file, the name and length of URI, checks the method  
-checks the status of the file pointed by the uri  
- __PUT__: statement, if the number of bytes read doesn't equal the number size of the buffer up to "\r\n\r\n". Any excess bytes from the message body will be stored in a seperate buffer of size(content_length).  
Afterwards, another while loop is implemented calling on the client FD recieving all the rest of the information.   
-the information is parsed and put into a buffer and eventually read to a file  
- __GET__: statement, head reads bytes from the uri, puts them into a buffer, after wards, sends the information to the client in chunks of 4096. Not all at once. the file is closed and moves on.   
- __HEAD__: statement, it only reads the file size of the file pointed by the uri using the stat command. 

Throughout every sendresponse call, I close the socket, free any heap memory, and continue with the while loop and it will infinetley wait for another connection. 

__Warning__: I was not sure how to implement my own bash script correctly. I wasn't able to implement a port number to run on so I'm using a file from test_files called port.sh which generates a port. Without that file, my scripts won't run. 
