#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char *word;
    struct WordProbability *prob_list;
    int counter;
    int sizeOfProbList;
    //... Add your own fields here
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;
    int counter;

} WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data) {
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        return 1;
    }
    *new_node = (Node){
        data,
        NULL};

    if (link_list->first == NULL) {
        link_list->first = new_node;
        link_list->last = new_node;
    } else {
        link_list->last->next = new_node;
        link_list->last = new_node;
    }

    link_list->size++;
    return 0;
}
/*************************************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    int rando = rand() % max_number;
    return rando;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary) {
    int numOfSteps = get_random_number(dictionary->size);  //Like the steps we'd like to check out on the list
    Node *ptr = dictionary->first;                         //point to the first node in the linked list
    int counter = 0;
    while (ptr != NULL) {
        counter++;
        if (counter - 1 == numOfSteps) {
            if (ptr->data->word[strlen(ptr->data->word) - 1] != '.') {  //If the random number in the list is different from a '.'
                return ptr->data;
            } else {  //if word is '.'
                counter = 0;
                ptr = dictionary->first;
                numOfSteps = get_random_number(dictionary->size);
                continue;
            }
        }
        ptr = ptr->next;
    }
    return NULL;
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr) {
    int totalRandom = 0;
    for (int i = 0; i < word_struct_ptr->sizeOfProbList; i++) {
        totalRandom = totalRandom + word_struct_ptr->prob_list[i].counter;
    }
    int randNum;
    if (totalRandom > 0) {
        randNum = get_random_number(totalRandom);
    } else {
        randNum = totalRandom;
    }
    int totalNum = 0;
    for (int i = 0; i < totalRandom; i++) {
        totalNum += word_struct_ptr->prob_list[i].counter;
        if (totalNum > randNum) {
            return word_struct_ptr->prob_list[i].word_struct_ptr;
        }
    }
    return NULL;
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary) {
    WordStruct *first = get_first_random_word(dictionary);
    int counter = 0;
    printf("%s", first->word);
    while (counter < MAX_WORDS_IN_SENTENCE_GENERATION) {
        WordStruct *nextiWord = get_next_random_word(first);
        if (nextiWord != NULL) {
            if (nextiWord->word[strlen(nextiWord->word) - 1] == '.') {
                printf(" %s", nextiWord->word);
                break;
            }
            printf(" %s", nextiWord->word);
            counter++;
            first = nextiWord;
        } else {
            break;
        }
    }
    return counter;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word, WordStruct *second_word) {
    if (first_word->prob_list == NULL) {
        first_word->prob_list = (WordProbability *)malloc(sizeof(WordProbability));  //If the array doesn't exist the space for it is allocated
        if (first_word->prob_list == NULL) {
            printf("Allocation Failure: no memory allocated\n");
            exit(EXIT_FAILURE);
        }
        first_word->sizeOfProbList = 0;
    }

    for (int i = 0; i < first_word->sizeOfProbList; i++) {  //if the word is exist in the list
        if (strcmp(first_word->prob_list[i].word_struct_ptr->word, second_word->word) == 0) {
            first_word->prob_list[i].counter++;
            return 0;
        }
    }
    if (first_word->sizeOfProbList != 0) {  //If not a first word we would like to expand the array
        first_word->prob_list = realloc(first_word->prob_list, sizeof(WordProbability) * (first_word->sizeOfProbList + 1));
        if (first_word->prob_list == NULL) {
            printf("Allocation Failure: no memory allocated\n");
            exit(EXIT_FAILURE);
        }
    }
    WordProbability secProArray = {second_word, 1};
    first_word->prob_list[first_word->sizeOfProbList] = secProArray;  //Introducing the word itself into the array
    first_word->sizeOfProbList++;
    return 1;
}

/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary) {
    char *wordi = (char *)malloc(sizeof(char) * (MAX_SENTENCE_LENGTH + 1));  // creat string that get the lines
    if (wordi == NULL) {                                                     //cheak that the allocation id done
        printf("Allocation Failure: no memory allocated\n");
        exit(EXIT_FAILURE);
    }
    int check = 0;  //boolean value
    if (words_to_read == -1)
        check = 1;
    int counter = 0;
    WordStruct *lastPointer = NULL;                          //the least point is equal null
    while (fgets(wordi, MAX_SENTENCE_LENGTH, fp) != NULL) {  //gets all the lines
        wordi[strlen(wordi) - 1] = '\0';
        char *token = strtok(wordi, " ");                              //take the firs word in the line
        while (token != NULL && (counter < words_to_read || check)) {  //break the line to words
            Node *curr = dictionary->first;                            //creat new node
            int found = 0;
            while (curr != NULL) {                                                                         //
                if (strcmp(curr->data->word, token) == 0) {                                                //if the word is exists Count it
                    if (lastPointer != NULL && lastPointer->word[strlen(lastPointer->word) - 1] != '.') {  //We'll go over the list
                        add_word_to_probability_list(lastPointer, curr->data);                             //If it is not a first word add
                    }
                    lastPointer = curr->data;
                    curr->data->counter++;
                    found = 1;
                    break;
                }
                curr = curr->next;
            }
            if (!found) {                                                            //if the word is not found creat wordstruct and count=1 and add to dictionary
                WordStruct *wordiStruct = (WordStruct *)malloc(sizeof(WordStruct));  //pinter to wordStruct
                if (wordiStruct == NULL) {
                    printf("Allocation Failure: no memory allocated\n");
                    exit(EXIT_FAILURE);
                }
                wordiStruct->word = (char *)malloc(sizeof(char) * (strlen(token) + 1));  //allocation to word and copy token
                if (wordiStruct->word == NULL) {
                    printf("Allocation Failure: no memory allocated\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(wordiStruct->word, token);
                wordiStruct->counter = 1;
                wordiStruct->sizeOfProbList = 0;
                wordiStruct->prob_list = NULL;
                add(dictionary, wordiStruct);  //add to the dictionary
                if (lastPointer != NULL) {
                    add_word_to_probability_list(lastPointer, wordiStruct);
                }

                lastPointer = wordiStruct;
            }
            token = strtok(NULL, " ");
            counter++;
        }
    }
    free(wordi);
}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary) {
    Node *freeAll = dictionary->first;
    Node *freeNext;
    while (freeAll != NULL) {
        freeNext = freeAll;
        freeAll = freeAll->next;
        free(freeNext->data->prob_list);
        free(freeNext->data->word);
        free(freeNext->data);
        free(freeNext);
    }
    free(dictionary);
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        printf("Usage: tweetsGenerator <seed> <number of sentences to generate> <path to file> <optional - number of words to read>\n");
        return EXIT_FAILURE;
    }

    int seed = (atoi(argv[1]));
    srand(seed);

    FILE *fd;
    fd = fopen(argv[3], "r");  //read from the text
    if (fd == NULL) {          //Check if the file exists or have an acceess
        printf("Error: this file doesn't exist or not access to the file");
        exit(EXIT_FAILURE);
    }
    LinkList *dictionary = (LinkList *)malloc(sizeof(LinkList));
    if (dictionary == NULL) {
        printf("Allocation Failure: no memory allocated\n");
        exit(EXIT_FAILURE);
    }
    dictionary->first = NULL;
    dictionary->last = NULL;
    dictionary->size = 0;

    if (argc == 4) {
        fill_dictionary(fd, -1, dictionary);
    } else {
        fill_dictionary(fd, atoi(argv[4]), dictionary);
    }

    for (int i = 1; i <= atoi(argv[2]); i++) {
        printf("Tweet %d: ", i);
        generate_sentence(dictionary);
        printf("\n");
    }

    fclose(fd);
    free_dictionary(dictionary);

    return 0;
}