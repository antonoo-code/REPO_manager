#include <stdio.h>
#include <string.h>

#define FILENAME "repositories.txt"  // Tiedoston nimi mitä käytetään ja mihin tallennetaan
#define MAX_ALIAS 256 //max merkkimäärä
#define MAX_LINK 256 //max merkkimäärä
#define MAX_ROW 512 //max merkkimäärä
#define MAX_ENTRIES 100 //max merkkimäärä
#define COLUMN_GAP 2 //max merkkimäärä
#define SHOW_ALL_KEYWORD "all"  //all on varattu sana jolla saa kaikki aliakset eikä etsi aliaksista.
#define ALIAS_HEADER "Alias"
#define LINK_HEADER "Link"

#define LINE_OK 0 // read_linen palautusarvot ni ei tarttee muistaa
#define LINE_TOO_LONG 1 
#define LINE_END 2 

char aliases[MAX_ENTRIES][MAX_ALIAS]; //globaalit taulukot
char links[MAX_ENTRIES][MAX_LINK];//globaalit taulukot
char input_line[MAX_ROW]; //puskuri inputille
char command[MAX_ROW];
char first_argument[MAX_ROW];
char second_argument[MAX_ROW];
char extra_argument[MAX_ROW];
int entry_count = 0; //montako merkitää on tallessa tällä hetkellä

int read_line(char line[], FILE *stream) // lukee rivin jos liian pitkä discardaa loput ja palauttaa "linetoolong"
{
    char discarded[MAX_ROW];
    int length;
    int line_too_long = 0;




    if (fgets(line, MAX_ROW, stream) == NULL) { //NULL = tiedosto on luettu
        return LINE_END;
    }

    length = strlen(line); // jos lopussa on '\n' mahtuu puskuriin
    if (length > 0 && line[length - 1] == '\n') {
        line[length - 1] = '\0';
        return LINE_OK; // rivi jatkuu
    }



    while (fgets(discarded, MAX_ROW, stream) != NULL) {
        length = strlen(discarded);
        if (strcmp(discarded, "\n") != 0) {
            line_too_long = 1;
        }
        if (length > 0 && discarded[length - 1] == '\n') {
            break;
        }
    }




    if (line_too_long) { // tarkistaa  onko liian pitkä
        return LINE_TOO_LONG;
    }
    return LINE_OK;
}

int is_valid_alias(const char *alias)
{
    int alias_length = strlen(alias);

    if (alias_length >= MAX_ALIAS) { // tarkistaa onko liian pitkä
        printf("Error: alias is too long (max %d characters).\n", MAX_ALIAS - 1);
        return 0;
    }
    if (strcmp(alias, SHOW_ALL_KEYWORD) == 0) {
        printf("Error: '%s' is reserved and cannot be used as an alias.\n", SHOW_ALL_KEYWORD);
        return 0;
    }
    return 1;
}



int is_valid_link(const char *link) // etsii aliaksen
{
    int link_length = strlen(link);

    if (link_length >= MAX_LINK) {
        printf("Error: repository link is too long (max %d characters).\n", MAX_LINK - 1);
        return 0;
    }
    return 1;
}

int find_alias(const char *alias)
{
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(aliases[i], alias) == 0) {
            return i;
        }
    }
    return -1;
}

void save_to_file(void) //kirjoittaa listan tiedostoon "w" tyhjentää vanhat poijes
{
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        printf("Error: could not open file '%s' for saving.\n", FILENAME);
        return;
    }

    for (int i = 0; i < entry_count; i++) {
        fprintf(file, "%s %s\n", aliases[i], links[i]); 
    }

    fclose(file);
}

void load_from_file(void) // lukee tallennetut aliakset
{
    char file_line[MAX_ROW];
    char alias[MAX_ROW];
    char link[MAX_ROW];
    char extra_field[MAX_ROW];
    int line_number = 0;
    int line_status;
    int field_count;

    FILE *file = fopen(FILENAME, "r"); // jos tiedostoa ei ole vielä ei virhe
    if (file == NULL) {
        return;
    }

    line_status = read_line(file_line, file);
    while (line_status != LINE_END) {
        line_number++;

        if (line_status == LINE_TOO_LONG) {
            printf("Error: line %d in '%s' is too long (max %d characters), line skipped.\n",
                   line_number, FILENAME, MAX_ROW - 1);
        } else {
            field_count = sscanf(file_line, "%s %s %s", alias, link, extra_field); // kolomas %s huomaa jos on liikaa sanoja

            if (field_count != 2) {
                printf("Error: line %d in '%s' must contain exactly an alias and a link "
                       "separated by a space, line skipped.\n", line_number, FILENAME);
            } else if (!is_valid_alias(alias) || !is_valid_link(link)) {
                printf("Error: line %d in '%s' contains invalid data, line skipped.\n",
                       line_number, FILENAME);
            } else if (find_alias(alias) != -1) {
                printf("Error: line %d in '%s': alias '%s' appears more than once, line skipped.\n",
                       line_number, FILENAME, alias);
            } else if (entry_count >= MAX_ENTRIES) { // lista täynnä
                printf("Error: '%s' contains more than %d entries, line %d and the rest are ignored.\n",
                       FILENAME, MAX_ENTRIES, line_number);
                break;
            } else {
                strcpy(aliases[entry_count], alias);
                strcpy(links[entry_count], link);
                entry_count++;
            }
        }

        line_status = read_line(file_line, file);
    }

    fclose(file);
}

int longest_alias_length(void) // etsii pisimmän aliaksen, ulkonäkö
{
    int longest_length = strlen(ALIAS_HEADER);

    for (int i = 0; i < entry_count; i++) {
        int alias_length = strlen(aliases[i]);
        if (alias_length > longest_length) {
            longest_length = alias_length;
        }
    }
    return longest_length;
}

void print_padded(const char *text, int column_width) // tulostaa siististi leveyden mukaan padding padding
{
    int text_length = strlen(text);

    printf("%s", text);
    for (int i = text_length; i < column_width; i++) {
        printf(" ");
    }
}

void cmd_add(const char *alias, const char *link) // add eka sit lisätään ja tallennetaan
{
    if (!is_valid_alias(alias) || !is_valid_link(link)) {
        return;
    }
    if (find_alias(alias) != -1) {
        printf("Error: alias '%s' already exists.\n", alias);
        return;
    }
    if (entry_count >= MAX_ENTRIES) {
        printf("Error: entry list is full (max %d entries).\n", MAX_ENTRIES);
        return;
    }

    strcpy(aliases[entry_count], alias);
    strcpy(links[entry_count], link);
    entry_count++;

    save_to_file();
    printf("Added '%s'.\n", alias);
}

void cmd_show(const char *alias) // all näyttää taulukkone
{
    if (strcmp(alias, SHOW_ALL_KEYWORD) == 0) {
        if (entry_count == 0) {
            printf("No entries.\n");
            return;
        }

        int alias_column_width = longest_alias_length() + COLUMN_GAP;

        print_padded(ALIAS_HEADER, alias_column_width);
        printf("%s\n", LINK_HEADER);
        for (int i = 0; i < entry_count; i++) {
            print_padded(aliases[i], alias_column_width);
            printf("%s\n", links[i]);
        }
    } else {
        int index = find_alias(alias);
        if (index == -1) {
            printf("Error: alias '%s' not found.\n", alias);
        } else {
            printf("%s\n", links[index]);
        }
    }
}

void cmd_list(void) // list pelkät aliakset
{
    if (entry_count == 0) {
        printf("No aliases.\n");
        return;
    }
    for (int i = 0; i < entry_count; i++) {
        printf("%s\n", aliases[i]);
    }
}

void cmd_delete(const char *alias) // poistaa 
{
    int index = find_alias(alias);
    if (index == -1) {
        printf("Error: alias '%s' not found.\n", alias);
        return;
    }

    for (int i = index; i < entry_count - 1; i++) { // siirtää ettei jää välejä 
        strcpy(aliases[i], aliases[i + 1]);
        strcpy(links[i], links[i + 1]);
    }
    entry_count--;

    save_to_file();
    printf("Deleted '%s'.\n", alias);
}

void cmd_help(void) // listaa kaikki komennot
{
    printf("Available commands:\n");
    printf("  add <alias> <repository link>  - add a new entry\n");
    printf("  show <alias>|all               - show one link or all links\n");
    printf("  list                           - list all aliases\n");
    printf("  delete <alias>                 - delete an entry\n");
    printf("  help                           - show this help\n");
    printf("  quit                           - quit the program\n");
}

int main(void)
{
    int line_status;
    int word_count;

    load_from_file(); // ladataan tiedot heti alussa

    while (1) { // main looppi
        printf("Enter a command (type help to see all commands): ");

        line_status = read_line(input_line, stdin);
        if (line_status == LINE_END) {
            break;
        }
        if (line_status == LINE_TOO_LONG) {
            printf("Error: input is too long (max %d characters).\n", MAX_ROW - 1);
            continue;
        }

        word_count = sscanf(input_line, "%s %s %s %s", //. jotta huomataa jos liikaa sanoja
                            command, first_argument, second_argument, extra_argument);

        if (word_count <= 0) { //tyhjä rivi
            continue;
        }
// katotaan mikä komento  ja paljon sanoja
        if (strcmp(command, "add") == 0) {
            if (word_count == 3) {
                cmd_add(first_argument, second_argument);
            } else {
                printf("Error: usage: add <alias> <repository link>\n");
            }
        }
        else if (strcmp(command, "show") == 0) {
            if (word_count == 2) {
                cmd_show(first_argument);
            } else {
                printf("Error: usage: show <alias>|all\n");
            }
        }
        else if (strcmp(command, "list") == 0) {
            if (word_count == 1) {
                cmd_list();
            } else {
                printf("Error: usage: list\n");
            }
        }
        else if (strcmp(command, "delete") == 0) {
            if (word_count == 2) {
                cmd_delete(first_argument);
            } else {
                printf("Error: usage: delete <alias>\n");
            }
        }
        else if (strcmp(command, "help") == 0) {
            if (word_count == 1) {
                cmd_help();
            } else {
                printf("Error: usage: help\n");
            }
        }
        else if (strcmp(command, "quit") == 0) {
            if (word_count == 1) {
                break;
            } else {
                printf("Error: usage: quit\n");
            }
        }
        else {
            printf("Error: unknown command '%s'. Type help for a list.\n", command);
        }
    }

    save_to_file(); // varmistus tallennus
    return 0;
}
