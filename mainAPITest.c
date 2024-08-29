#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "cJSON.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <curl/curl.h>

#define dollarUrl "https://api.frankfurter.app/latest?from=USD"
#define goldUrl "https://api.gold-api.com/price/XAU"
#define PORT 8080

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct parameters {
	float deposit;
	float weight;
	float stackvalue;
	float depositPotential;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

void apiFetch(int i, char* output) {
	CURL *curl_handle;
	  CURLcode res;

	  struct MemoryStruct chunk;

	  chunk.memory = malloc(1);  /* grown as needed by the realloc above */
	  chunk.size = 0;    /* no data at this point */

	  curl_global_init(CURL_GLOBAL_ALL);

	  /* init the curl session */
	  curl_handle = curl_easy_init();

	  /* specify URL to get */
	  if (i) {
		  curl_easy_setopt(curl_handle, CURLOPT_URL, dollarUrl);
	  }
	  else {
		  curl_easy_setopt(curl_handle, CURLOPT_URL, goldUrl);
	  }


	  /* send all data to this function  */
	  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	  /* we pass our 'chunk' struct to the callback function */
	  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	  /* some servers do not like requests that are made without a user-agent
	     field, so we provide one */
	  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	  /* get it! */
	  res = curl_easy_perform(curl_handle);

	  /* check for errors */
	  if(res != CURLE_OK) {
	    fprintf(stderr, "curl_easy_perform() failed: %s\n",
	            curl_easy_strerror(res));
	  }
	  else {


	    /*
	     * Now, our chunk.memory points to a memory block that is chunk.size
	     * bytes big and contains the remote file.
	     *
	     * Do something nice with it!
	     */

	    printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
	    printf("%s", chunk.memory);
	    strcpy(output, chunk.memory);
	  }

	  /* cleanup curl stuff */
	  curl_easy_cleanup(curl_handle);

	  free(chunk.memory);

	  /* we are done with libcurl, so clean it up */
	  curl_global_cleanup();
}

float getGoldPrice() {
	char smallBuffer[500]={0};
	char bigBuffer[500]={0};

	apiFetch(0, smallBuffer);
	apiFetch(1, bigBuffer);

	cJSON * generalInfo = cJSON_Parse(smallBuffer);
	cJSON * detailedInfo = cJSON_Parse(bigBuffer);

	cJSON *gold = cJSON_GetObjectItemCaseSensitive(generalInfo, "price");

	// Retrieve the value as a float by casting from double
	float goldPriceUSD = (float)gold->valuedouble;


	cJSON * rates = cJSON_GetObjectItemCaseSensitive(detailedInfo, "rates");
	cJSON * usdsek = cJSON_GetObjectItemCaseSensitive(rates, "SEK");
	// Retrieve the value as a float by casting from double
	float rate = (float)usdsek->valuedouble;

	float goldPriceSEK = goldPriceUSD * rate;
	goldPriceSEK = goldPriceSEK/31.1;

	return goldPriceSEK;
}

int serverStart() {


}

int main(void)
{

	struct parameters values;

	FILE *file;

	file = fopen("count.txt", "r");
	    if (file == NULL) {
	        perror("Error opening file");
	        return EXIT_FAILURE;
	    }
	    if (fscanf(file, "%f", &values.weight) != 1) {
	            perror("Error reading first float");
	            fclose(file);
	            return EXIT_FAILURE;
	        }
	    if (fscanf(file, "%f", &values.deposit) != 1) {
	    	   	perror("Error reading first float");
	    	    fclose(file);
	    	    return EXIT_FAILURE;
	    	}
	//char tmp[500];
	//apiFetch(0, &tmp);
	//apiFetch(1, &tmp);
	float spot = getGoldPrice();
	values.stackvalue = spot * values.weight;
	values.depositPotential = values.deposit/spot;
	printf("\n Gold price:  %f \n", spot);
	printf("\n stack worth %f sek", values.stackvalue);
	printf("\n deposit worth %f grams gold", values.depositPotential);
	printf("\n deposit of %f SEK", values.deposit);
	printf("\n stack is %f grams gold", values.weight);

	//server
	int server_fd, new_socket;
	    struct sockaddr_in address;
	    int opt = 1;
	    int addrlen = sizeof(address);

	    // Creating socket file descriptor
	    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	        perror("socket failed");
	        return -1;
	    }

	    // Forcefully attaching socket to the port 8080
	    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
	        perror("setsockopt failed");
	        return -1;
	    }

	    address.sin_family = AF_INET;
	    address.sin_addr.s_addr = INADDR_ANY;
	    address.sin_port = htons(PORT);

	    // Bind the socket to the network address and port
	    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
	        perror("bind failed");
	        return -1;
	    }

	    // Start listening for incoming connections
	    if (listen(server_fd, 3) < 0) {
	        perror("listen failed");
	        return -1;
	    }

	    printf("Server is listening on port %d...\n", PORT);

	    // Enter an infinite loop to keep the server running
	    while (1) {
	        // Accept an incoming connection
	        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
	            perror("accept failed");
	            return -1;
	        }

	        // Initialize and send the struct


	        if (send(new_socket, &values, sizeof(values), 0) < 0) {
	            perror("send failed");
	            close(new_socket);
	            continue;  // Continue listening for more connections
	        }

	        printf("Data sent to the client.\n");

	        close(new_socket);  // Close the connection with the current client
	    }

	    // Close the server socket (this line will never be reached in this example)
	    close(server_fd);

}

