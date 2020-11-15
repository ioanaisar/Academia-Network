/* Copyright Isar Ioana-Teodora, Turcu Arina-Emanuela 313CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./publications.h"

#define HMAX 20000
#define NMAX2 2000

struct author {
    char *name;
    int64_t id;
    char *institution;
    struct LinkedList *colab;
    int visited;
    int distance;
};

struct article {
    char *title;
    char *venue;
    int year;
    struct author **authors;
    int num_authors;
    char **fields;
    int num_fields;
    int64_t id;
    int64_t *references;
    int num_refs;
    int visited; /* imi foloseste la bfs */
    int dist;    /* imi foloseste la cerinta 3 */
};

struct publications_data {
    struct Hashtable *ht;
    struct Hashtable *ht_cit;
    struct Hashtable *ht_venue;
    struct Hashtable *ht_influence;
    struct Hashtable *ht_field;
    struct Hashtable *ht_authors; /* autori si colaboratori */
};

char *strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) {
        memcpy(p, s, size);
    }
    return p;
}

PublData *init_publ_data(void) {
    struct publications_data *data;

    data = malloc(sizeof(struct publications_data));
    data->ht = malloc(sizeof(struct Hashtable));
    data->ht_cit = malloc(sizeof(struct Hashtable));
    data->ht_venue = malloc(sizeof(struct Hashtable));
    data->ht_influence = malloc(sizeof(struct Hashtable));
    data->ht_field = malloc(sizeof(struct Hashtable));
    data->ht_authors = malloc(sizeof(struct Hashtable));

    DIE(data == NULL, "data");
    DIE(data->ht == NULL, "data->ht");
    DIE(data->ht_cit == NULL, "data->ht_cit");
    DIE(data->ht_venue == NULL, "data->ht_venue");
    DIE(data->ht_influence == NULL, "data->ht_influence");
    DIE(data->ht_field == NULL, "data->ht_field");
    DIE(data->ht_authors == NULL, "data->ht_authors");

    init_ht(data->ht, HMAX, hash_function_int, compare_function_ints);
    init_ht(data->ht_cit, HMAX, hash_function_int, compare_function_ints);
    init_ht(data->ht_venue, HMAX, hash_function_string,
        compare_function_strings);
    init_ht(data->ht_influence, HMAX, hash_function_int,
        compare_function_ints);
    init_ht(data->ht_field, HMAX, hash_function_string,
        compare_function_strings);
    init_ht(data->ht_authors, HMAX * 2, hash_function_int,
        compare_function_ints);

    return data;
}

void __remove_ht_entry(struct Hashtable *ht, void *key) {
    int index, i, j;
    struct LinkedList *buck, *list;
    struct Node *entry;

    index = ht->hash_function(key) % ht->hmax;
    buck = &ht->buckets[index];

    entry = buck->head;
    i = 0;

    while (entry != NULL) {
        if (ht->compare_function(((struct info *)entry->data)->key, key) == 0) {
            free(((struct info *)entry->data)->key);

            if (((struct info *)entry->data)->value != NULL) {
                free(((struct article *)((struct info *)entry->data)->value)
                        ->title);
                free(((struct article *)((struct info *)entry->data)->value)
                        ->venue);
                free(((struct article *)((struct info *)entry->data)->value)
                        ->references);

                for (j = 0; j < ((struct article *)((struct info *)entry->data)
                            ->value)->num_authors; j++) {
                    free(((struct article *)((struct info *)entry->data)->value)
                             ->authors[j]
                             ->name);
                    free(((struct article *)((struct info *)entry->data)->value)
                             ->authors[j]
                             ->institution);

                    list = ((struct article *)((struct info *)entry->data)
                                ->value)->authors[j]->colab;
                    if (list != NULL)
                        free_list(&list);

                    free(((struct article *)((struct info *)entry->data)->value)
                             ->authors[j]);
                }
                free(((struct article *)((struct info *)entry->data)
                            ->value)->authors);

                for (j = 0; j < ((struct article *)((struct info *)entry->data)
                            ->value)->num_fields; j++) {
                    free(((struct article *)((struct info *)entry->data)->value)
                             ->fields[j]);
                }
                free(((struct article *)((struct info *)entry->data)
                                ->value)->fields);

                free(((struct info *)entry->data)->value);
            }

            free(entry->data);
            entry = remove_nth_node(buck, i);
            free(entry);

            break;
        }

        i++;
        entry = entry->next;
    }
}

void __remove_ht_cit_entry(struct Hashtable *ht, void *key) {
    int index, i;
    struct LinkedList *buck;
    struct Node *entry;

    index = ht->hash_function(key) % ht->hmax;
    buck = &ht->buckets[index];

    entry = buck->head;
    i = 0;

    while (entry != NULL) {
        if (ht->compare_function(((struct info *)entry->data)->key, key) == 0) {
            free(((struct info *)entry->data)->key);
            free(((struct info *)entry->data)->value);

            free(entry->data);
            entry = remove_nth_node(buck, i);
            free(entry);

            break;
        }

        i++;
        entry = entry->next;
    }
}

void __remove_ht_venue_entry(struct Hashtable *ht, void *key) {
    int index, i;
    struct LinkedList *buck;
    struct Node *entry;
    struct LinkedList *list;

    index = ht->hash_function(key) % ht->hmax;

    buck = &ht->buckets[index];

    entry = buck->head;
    i = 0;

    while (entry != NULL) {
        if (ht->compare_function(((struct info *)entry->data)->key,
                    (char *)key) == 0) {
            free(((struct info *)entry->data)->key);
            list = (struct LinkedList *)((struct info *)entry->data)->value;
            free_list(&list);

            free(entry->data);
            entry = remove_nth_node(buck, i);
            free(entry);

            break;
        }

        i++;
        entry = entry->next;
    }
}

void __remove_ht_auth_entry(struct Hashtable *ht, void *key) {
    int index, i, j;
    struct LinkedList *buck, *list;
    struct Node *entry;

    index = ht->hash_function(key) % ht->hmax;
    buck = &ht->buckets[index];

    entry = buck->head;
    i = 0;

    while (entry != NULL) {
        if (ht->compare_function(((struct info *)entry->data)->key, key) == 0) {
            free(((struct info *)entry->data)->key);

            free(entry->data);
            entry = remove_nth_node(buck, i);
            free(entry);

            break;
        }

        i++;
        entry = entry->next;
    }
}

void __free_ht(struct Hashtable *ht,
               void (*rem_entry)(struct Hashtable *, void *)) {
    int i;
    struct LinkedList *buck;
    void *key;

    for (i = 0; i < ht->hmax; ++i) {
        buck = &(ht->buckets[i]);

        while (buck->size > 0) {
            key = ((struct info *)buck->head->data)->key;
            rem_entry(ht, key);
        }
    }

    free(ht->buckets);
    free(ht);
}

void destroy_publ_data(PublData *data) {
    __free_ht(data->ht, __remove_ht_entry);
    __free_ht(data->ht_cit, __remove_ht_cit_entry);
    __free_ht(data->ht_venue, __remove_ht_venue_entry);
    __free_ht(data->ht_influence,
              __remove_ht_venue_entry); /* functioneaza la fel */
    __free_ht(data->ht_field, __remove_ht_venue_entry);
    __free_ht(data->ht_authors, __remove_ht_auth_entry);

    free(data);
}

void add_paper(PublData *data2, const char *title, const char *venue,
               const int year, const char **author_names,
               const int64_t *author_ids, const char **institutions,
               const int num_authors, const char **fields, const int num_fields,
               const int64_t id, const int64_t *references,
               const int num_refs) {
    struct article *paper = malloc(sizeof(struct article));
    int *ref;
    int i;
    struct LinkedList *pt_venue, *pt_field;
    struct author *pt_auth;
    struct LinkedList *tmp;
    int *num_cit;
    int j;
    char camp[2000];

    paper->title = strdup(title);
    paper->venue = strdup(venue);
    paper->year = year;
    paper->num_authors = num_authors;
    paper->num_fields = num_fields;
    paper->id = id;
    paper->num_refs = num_refs;
    paper->visited = 0;
    paper->dist = 0x7fffffff;

    paper->authors = malloc(num_authors * sizeof(struct author *));
    for (i = 0; i < num_authors; i++) {
        paper->authors[i] = malloc(sizeof(struct author));
        DIE(paper->authors[i] == NULL, "paper->authors[i]");

        paper->authors[i]->name = strdup(author_names[i]);
        DIE(paper->authors[i]->name == NULL, "paper->authors[i]->name");

        paper->authors[i]->id = author_ids[i];

        paper->authors[i]->institution = strdup(institutions[i]);
        DIE(paper->authors[i]->institution == NULL,
            "paper->authors[i]->institution");

        paper->authors[i]->visited = 0;
        paper->authors[i]->distance = 0;
        paper->authors[i]->colab = malloc(sizeof(struct LinkedList));
        DIE(paper->authors[i]->colab == NULL, "paper->authors[i]->colab");

        init_list(paper->authors[i]->colab);
    }

    for (i = 0; i < num_authors; ++i) {
        /* adaug in ht_authors */
        pt_auth = get(data2->ht_authors, (void *)&author_ids[i]);

        for (j = 0; j < num_authors; ++j) {
            if (j != i) {
                add_nth_node(paper->authors[i]->colab, 0,
                             (void *)&paper->authors[j]->id);
            }
        }

        if (pt_auth == NULL) {
            put(data2->ht_authors, (void *)&(paper->authors[i]->id),
                sizeof(int64_t), paper->authors[i]);
        } else {
            for (j = 0; j < num_authors; ++j) {
                if (j != i) {
                    add_nth_node(pt_auth->colab, 0,
                        (void *)&paper->authors[j]->id);
                }
            }
        }
    }

    paper->fields = malloc(num_fields * sizeof(char *));
    for (i = 0; i < num_fields; i++) {
        paper->fields[i] = strdup(fields[i]);
    }

    /* adaug in ht_cit si in ht_influence */
    paper->references = malloc(num_refs * sizeof(int64_t));
    for (i = 0; i < num_refs; i++) {
        paper->references[i] = references[i];

        /* ht_cit */
        num_cit = (int *)malloc(sizeof(int));
        ref = get(data2->ht_cit, &paper->references[i]);

        if (ref == NULL) {
            *num_cit = 1;
            put(data2->ht_cit, &paper->references[i], sizeof(int64_t), num_cit);
        } else {
            *num_cit = (*ref) + 1;
            put(data2->ht_cit, &paper->references[i], sizeof(int64_t), num_cit);
        }

        /* ht_influence */
        tmp = get(data2->ht_influence, &paper->references[i]);
        if (tmp == NULL) {
            struct LinkedList *list = malloc(sizeof(struct LinkedList));
            DIE(list == NULL, "list in add_paper");

            init_list(list);
            add_nth_node(list, 0, paper);
            put(data2->ht_influence, &paper->references[i],
                sizeof(int64_t), list);
        } else {
            add_nth_node(tmp, 0, paper);
        }
    }

    /* adaug in ht_venue */
    pt_venue = get(data2->ht_venue, paper->venue);

    if (pt_venue == NULL) {
        struct LinkedList *list = malloc(sizeof(struct LinkedList));
        DIE(list == NULL, "list in add_paper");

        init_list(list);
        add_nth_node(list, 0, paper);
        put(data2->ht_venue, paper->venue, strlen(paper->venue) + 1, list);
    } else {
        add_nth_node(pt_venue, 0, paper);
    }

    /* adaug in ht_field */
    for (i = 0; i < num_authors; i++) {
        for (j = 0; j < num_fields; j++) {
            memset(camp, '\0', 2000);
            strncpy(camp, paper->authors[i]->institution, sizeof(camp));
            strncat(camp, paper->fields[j], sizeof(camp));

            pt_field = get(data2->ht_field, camp);

            if (pt_field == NULL) {
                struct LinkedList *list = malloc(sizeof(struct LinkedList));
                DIE(list == NULL, "list in add_paper");

                init_list(list);
                add_nth_node(list, 0, paper);
                put(data2->ht_field, &camp, strlen(camp) + 1, list);
            } else {
                add_nth_node(pt_field, 0, paper);
            }
        }
    }

    /* adaug in ht */
    put(data2->ht, &paper->id, sizeof(int64_t), paper);
}

void __restore_visited(struct LinkedList *visited_papers) {
    struct Node *curr;
    int i;

    for (i = 0; i < visited_papers->size; ++i) {
        curr = visited_papers->head;

        while (curr != NULL) {
            ((struct article *)curr->data)->visited = 0;
            curr = curr->next;
        }
    }
}

void __restore_visited_ath(struct LinkedList *visited_authors) {
    struct Node *curr;
    int i;

    for (i = 0; i < visited_authors->size; ++i) {
        curr = visited_authors->head;

        while (curr != NULL) {
            ((struct author *)curr->data)->visited = 0;
            curr = curr->next;
        }
    }
}

void __restore_distances(struct LinkedList *distances_papers) {
    struct Node *curr;
    int i;

    for (i = 0; i < distances_papers->size; ++i) {
        curr = distances_papers->head;

        while (curr != NULL) {
            ((struct article *)curr->data)->dist = 0;
            curr = curr->next;
        }
    }
}

void __restore_distances_ath(struct LinkedList *distances_authors) {
    struct Node *curr;
    int i;

    for (i = 0; i < distances_authors->size; ++i) {
        curr = distances_authors->head;

        while (curr != NULL) {
            ((struct author *)curr->data)->distance = 0;
            curr = curr->next;
        }
    }
}

/* returneaza numarul de paper-uri care au printre referinte pe cel dat ca
 * parametru */
static int __get_importance(PublData *data2, struct article *ref_paper) {
    int *num = get(data2->ht_cit, &ref_paper->id);

    if (num == NULL) {
        return 0;
    }

    return *num;
}

static struct article *__dependence(PublData *data, struct article *paper) {
    struct article *ref;
    struct article *smth;
    int i;
    int oldest_year = 0xfffffff;
    struct article *oldest_paper = NULL;
    struct Queue *q;

    struct LinkedList *visited_papers;
    visited_papers = malloc(sizeof(struct LinkedList));
    DIE(visited_papers == NULL, "visited_papers");

    init_list(visited_papers);

    q = malloc(sizeof(struct Queue));
    DIE(q == NULL, "q malloc");
    init_q(q);

    paper->visited = 1;
    add_nth_node(visited_papers, 0xfffffff, paper);
    enqueue(q, paper);

    while (!is_empty_q(q)) {
        smth = (struct article *)front(q);
        dequeue(q);

        for (i = 0; i < smth->num_refs; ++i) {
            ref = get(data->ht, &smth->references[i]);

            if (ref != NULL && ref->visited == 0) {
                ref->visited = 1;
                add_nth_node(visited_papers, 0xfffffff, ref);

                if (ref->year < oldest_year) {
                    oldest_paper = ref;
                    oldest_year = ref->year;
                } else if (ref->year == oldest_year) {
                    int ref_imp = __get_importance(data, ref);
                    int old_imp = __get_importance(data, oldest_paper);

                    if (ref_imp > old_imp) {
                        oldest_paper = ref;
                    } else if (ref_imp == old_imp) {
                        if (ref->id < oldest_paper->id) {
                            oldest_paper = ref;
                        }
                    }
                }
                enqueue(q, ref);
            }
        }
    }
    purge_q(q);
    free(q);

    __restore_visited(visited_papers);

    free_list(&visited_papers);

    return oldest_paper;
}

char *get_oldest_influence(PublData *data, const int64_t id_paper) {
    char *none = "None";
    int64_t p_id = id_paper;

    struct article *paper;
    struct article *oldest_paper;

    paper = get(data->ht, &p_id);
    oldest_paper = NULL;

    if (paper == NULL) {
        return none;
    }

    oldest_paper = __dependence(data, paper);

    if (oldest_paper == NULL) {
        return none;
    }

    return oldest_paper->title;
}

float get_venue_impact_factor(PublData *data, const char *venue) {
    struct Node *curr;
    struct LinkedList *list;

    int i;
    int num_cit = 0, num_papers;

    list = get(data->ht_venue, (void *)venue);

    if (list == NULL) {
        return 0;
    }

    num_papers = list->size;

    curr = list->head;
    for (i = 0; i < list->size; i++) {
        num_cit += __get_importance(data, (struct article *)curr->data);
        curr = curr->next;
    }

    return (double)num_cit / (double)num_papers;
}

int get_number_of_influenced_papers(PublData *data, const int64_t id_paper,
                                    const int distance) {
    struct article *inf;
    struct article *tmp;
    int i;
    int contor = 0;

    struct article *paper;
    struct LinkedList *influencers;
    struct Node *node;
    struct Queue *q;

    struct LinkedList *visited_papers;
    struct LinkedList *distances_papers;
    visited_papers = malloc(sizeof(struct LinkedList));
    DIE(visited_papers == NULL, "visited");
    distances_papers = malloc(sizeof(struct LinkedList));
    DIE(distances_papers == NULL, "distance");

    init_list(visited_papers);
    init_list(distances_papers);

    q = malloc(sizeof(struct Queue));
    DIE(q == NULL, "q malloc");
    init_q(q);

    paper = get(data->ht, (void *)&id_paper);

    paper->visited = 1;
    paper->dist = 0;
    add_nth_node(visited_papers, 0, paper);
    enqueue(q, paper);

    while (!is_empty_q(q)) {
        tmp = (struct article *)front(q);
        dequeue(q);

        influencers = get(data->ht_influence, &tmp->id);
        if (influencers == NULL) {
            continue;
        }

        node = influencers->head;

        for (i = 0; i < influencers->size; i++) {
            inf = node->data;

            if (inf->visited == 0 && tmp->dist < distance) {
                inf->visited = 1;
                inf->dist = tmp->dist + 1;
                add_nth_node(visited_papers, 0, inf);
                add_nth_node(distances_papers, 0, inf);
                contor++;
                enqueue(q, inf);
            }
            node = node->next;
        }
    }
    purge_q(q);
    free(q);

    __restore_visited(visited_papers);
    __restore_distances(distances_papers);

    free_list(&visited_papers);
    free_list(&distances_papers);

    return contor;
}

int get_erdos_distance(PublData *data, const int64_t id1, const int64_t id2) {
    struct author *auth, *col;
    struct LinkedList *colab;

    struct author *tmp;
    int i, dd = -1, ok = 0;

    struct Node *node;
    struct Queue *q;

    struct LinkedList *visited_authors;
    struct LinkedList *distances_authors;
    visited_authors = malloc(sizeof(struct LinkedList));
    DIE(visited_authors == NULL, "visited");
    distances_authors = malloc(sizeof(struct LinkedList));
    DIE(distances_authors == NULL, "distance");

    init_list(visited_authors);
    init_list(distances_authors);

    q = malloc(sizeof(struct Queue));
    DIE(q == NULL, "q malloc");
    init_q(q);

    auth = get(data->ht_authors, (void *)&id1);

    auth->visited = 1;
    auth->distance = 0;
    add_nth_node(visited_authors, 0, auth);
    enqueue(q, auth);

    while (!is_empty_q(q)) {
        tmp = (struct author *)front(q);
        dequeue(q);

        colab = tmp->colab;
        if (colab->size == 0) {
            continue;
        }

        node = colab->head;

        col = get(data->ht_authors, node->data);

        for (i = 0; i < colab->size; i++) {
            col = get(data->ht_authors, node->data);

            if (col->id == id2 && ok == 0) {
                dd = tmp->distance + 1;
                ok = 1;
                purge_q(q);
                free(q);

                __restore_visited_ath(visited_authors);
                __restore_distances_ath(distances_authors);

                free_list(&visited_authors);
                free_list(&distances_authors);

                return dd;
            }

            if (col->visited == 0) {
                col->visited = 1;
                col->distance = tmp->distance + 1;
                add_nth_node(visited_authors, 0, col);
                add_nth_node(distances_authors, 0, col);
                enqueue(q, col);
            }
            node = node->next;
        }
    }
    purge_q(q);
    free(q);

    __restore_visited_ath(visited_authors);
    __restore_distances_ath(distances_authors);

    free_list(&visited_authors);
    free_list(&distances_authors);

    return dd;
}

char **get_most_cited_papers_by_field(PublData *data, const char *field,
                                      int *num_papers) {
    /* TODO: implement get_most_cited_papers_by_field */

    return NULL;
}

int get_number_of_papers_between_dates(PublData *data2, const int early_date,
                                       const int late_date) {
    int i, nr, years;
    struct Node *curr;
    struct LinkedList *buck;
    struct article *article2;
    void *value2;
    nr = 0;

    /* se parcurge un hashtable realizat dupa id */
    for (i = 0; i < HMAX; ++i) {
        buck = &(data2->ht->buckets[i]);

        if (buck->head != NULL) {
            curr = buck->head;

            while (curr != NULL) {
                /* key = ((struct info*)curr->data)->key; */
                value2 = ((struct info *)curr->data)->value;
                article2 = (struct article *)value2;
                years = article2->year;

                /* se verifica daca anul de publicatie este in
                 * intervalul dorit */
                if (years >= early_date && years <= late_date) {
                    nr++;
                }

                curr = curr->next;
            }
        }
    }

    return nr;
}

int get_number_of_authors_with_field(PublData *data2, const char *institution,
                                     const char *field) {
    char field2[NMAX2], **authorss;
    struct Node *curr;
    int ok, nr, k, o, nr_authors, ok2, j, i;
    struct LinkedList *list;
    struct article *articol;

    nr = 0;
    ok = 0;
    memset(field2, '\0', NMAX2);
    authorss = malloc(NMAX2 * sizeof(char *));
    DIE(authorss == NULL, "authorss in nr_auth");

    strncpy(field2, institution, NMAX2);
    strncat(field2, field, NMAX2);

    /* se intoarce lista aflata la pozitia din hashtable
     * data de institutie si domeniu */
    list = get(data2->ht_field, (void *)field2);

    if (list == NULL) {
        free(authorss);
        return 0;
    } else {
        curr = list->head;

        while (curr != NULL) {
            articol = (struct article *)curr->data;
            ok = 0;

            /* se verifica daca articolul este din domeniul cautat */
            for (j = 0; j < articol->num_fields && ok == 0; j++) {
                if (strcmp(articol->fields[j], field) == 0) {
                    ok = 1;
                }
            }

            nr_authors = articol->num_authors;

            /* se verifica ca autorul sa fie de la institutia dorita */
            for (k = 0; k < nr_authors && ok == 1; k++) {
                if (strcmp(articol->authors[k]->institution,
                        institution) == 0) {
                    ok2 = 0;

                    /* se verifica sa nu fi fost un autor numarat anterior   */
                    for (o = 0; o < nr && ok2 == 0; o++) {
                        if (strcmp(authorss[o],
                                articol->authors[k]->name) == 0) {
                            ok2 = 1;
                        }
                    }

                    if (ok2 == 0) {
                        authorss[nr] = strdup(articol->authors[k]->name);
                        nr++;
                    }
                }
                ok = 1;
            }
            curr = curr->next;
        }
    }

    for (i = 0; i < nr; i++) {
        free(authorss[i]);
    }

    free(authorss);

    return nr;
}

int *get_histogram_of_citations(PublData *data, const int64_t id_author,
                                int *num_years) {
    /* TODO: implement get_histogram_of_citations */

    *num_years = 0;

    return NULL;
}

char **get_reading_order(PublData *data, const int64_t id_paper,
                         const int distance, int *num_papers) {
    /* TODO: implement get_reading_order */

    *num_papers = 0;

    return NULL;
}

char *find_best_coordinator(PublData *data, const int64_t id_author) {
    /* TODO: implement find_best_coordinator */

    return NULL;
}
