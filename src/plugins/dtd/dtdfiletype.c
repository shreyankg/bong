/*
 * Copyright/Licensing information.
 */

#include "dtdfiletype.h"
#include "translatable.h"
#include "utils.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>

/* Public methods */

void dtd_file_type_read_file (DtdFileType *self, Translatable *tr, gchar *file_name)
{
    gchar *input_contents = NULL;
    GError *read_error = NULL;
    gsize length = -1;
    /* read file contents into an allocated string */
    g_file_get_contents(file_name, &input_contents, &length, &read_error);
    /* parse the contents */
    dtd_file_type_read_contents (self, tr, input_contents);
    /* free the string containing the file contents */
    g_free (input_contents);
}

void dtd_file_type_write_file (DtdFileType *self, Translatable *tr, gchar *file_name)
{
    gchar *input_contents = NULL;
    gchar *output_contents = NULL;
    GError *read_error = NULL;
    gsize length = -1;
    /* read the input file contents into an allocated string */
    g_file_get_contents(file_name, &input_contents, &length, &read_error);

    /* fetch the modified file contents into another allocated string.
     * This string is created by modifying the input contents according
     * to entries in the Translatable object
     */
    output_contents = dtd_file_type_write_contents (self, tr, input_contents);

    /* FIXME: for now, just printing the output contents.
     * Should write it to a new file
     */
    g_printf("%s\n", output_contents);

    /* free allocated strings after usage */
    g_free (input_contents);
    g_free(output_contents);
}

/* This is called when the class is initialized */
void dtd_file_type_class_init (gpointer klass, gpointer klass_data)
{
    FileTypeClass *parent_class = FILE_TYPE_CLASS (klass);

    /* setup the vtable, override parent functions */
    parent_class->read_file = (void *)(dtd_file_type_read_file);
    parent_class->read_contents = (void *)(dtd_file_type_read_contents);
    parent_class->write_file = (void *)(dtd_file_type_write_file);
    parent_class->write_contents = (void *)(dtd_file_type_write_contents);
}

/* this is the constructor */
void dtd_file_type_instance_init (GTypeInstance *instance, gpointer klass)
{
    /* not needed atm */
}

/* boilerplate code */
GType dtd_file_type_get_type (void)
{
    static GType type = 0;

    if (type == 0)
    {
        static const GTypeInfo type_info =
        {
            sizeof (DtdFileTypeClass),
            NULL,                   /* base_init */
            NULL,                   /* base_finalize */
            dtd_file_type_class_init,   /* class_init */
            NULL,                   /* class_final */
            NULL,                   /* class_data */
            sizeof (DtdFileType),
            0,                      /* n_preallocs */
            dtd_file_type_instance_init /* instance_init */
        };

        type = g_type_register_static (TYPE_FILE_TYPE, "DtdFileType", &type_info, 0);
    }

    return type;
}

void dtd_file_type_read_contents (DtdFileType *self, Translatable *tr, gchar *input_contents)
{
    int entry_number = -1;
    GError *error = 0;
    GMatchInfo *match_info;
    GRegex *string_regex = NULL, *note_regex = NULL;

    /* create new regex to search for localized strings */
    string_regex = g_regex_new("<!ENTITY\\s+?(.*)\\s*?\"(.*)\">", G_REGEX_UNGREEDY|G_REGEX_DOTALL, 0, &error);
    /* find all matches for the above regex */
    g_regex_match(string_regex, input_contents, 0, &match_info);
    /* for every match, */
    while (g_match_info_matches (match_info))
    {
        gchar *key = g_match_info_fetch(match_info, 1);
        gchar *value = g_match_info_fetch(match_info, 2);
        entry_number++;
        /* extract key (UIK) and value, store in Translatable object */
        translatable_add_entry (tr, entry_number, key, NULL, "en", value);
        g_match_info_next(match_info, NULL);

        g_free(key);
        g_free(value);
    }
    /* free regex variables */
    g_match_info_free(match_info);
    g_regex_unref(string_regex);

    /* create new regex for notes */
    note_regex = g_regex_new("<!--\\s*LOCALIZATION NOTE\\s*?\\((.*)\\)\\s*:\\s*?(.*)\\s*?-->", G_REGEX_UNGREEDY|G_REGEX_DOTALL, 0, &error);
    /* find all matches for the above regex */
    g_regex_match(note_regex, input_contents, 0, &match_info);
    /* for every match, */
    while (g_match_info_matches (match_info))
    {
        gchar *key = g_match_info_fetch(match_info, 1);
        /* tokenize the key (comma separated UIKs) */
        GList *key_list = find_elements_for_note(key, strlen(key));
        gchar *value = g_match_info_fetch(match_info, 2);
        GList *key_item = key_list;
        /* for every UIK */
        while (key_item != NULL)
        {
            /* store note for UIK */
            translatable_set_note_for_uik (tr, key_item->data, value);
            key_item = key_item->next;
        }
        /* free token list */
        g_list_free_full(key_list, g_free);

        g_free(key);
        g_free(value);
        g_match_info_next(match_info, NULL);
    }
    /* free regex variables */
    g_match_info_free(match_info);
    g_regex_unref(string_regex);
}

gchar* dtd_file_type_write_contents (DtdFileType *self, Translatable *tr, gchar *input_contents)
{
    GError *error = 0;
    GMatchInfo *match_info;
    GRegex *string_regex = NULL;
    int lastpos = 0;
    gchar *output_contents = g_strdup(""), *line, *tmp;

    /* create new regex to search for localized strings */
    string_regex = g_regex_new("<!ENTITY\\s+?(.*)\\s*?\"(.*)\">", G_REGEX_UNGREEDY|G_REGEX_DOTALL, 0, &error);
    /* find all matches for the above regex */
    g_regex_match(string_regex, input_contents, 0, &match_info);
    /* for every match, */
    while (g_match_info_matches (match_info))
    {
        /* add the part starting from the last match's ending position
         *to the new match's starting position to the output */
        int startpos, endpos;
        g_match_info_fetch_pos (match_info, 0, &startpos, &endpos);
        input_contents[startpos] = '\0';
        tmp = output_contents;
        output_contents = g_strjoin ("", output_contents, input_contents + lastpos, NULL);
        g_free (tmp);
        input_contents[startpos] = '<';
        lastpos = endpos;

        /* extract UIK, retrieve its value from Translatable object */
        gchar *key = g_match_info_fetch(match_info, 1);
        gchar *value = translatable_get_string_for_uik(tr, key, "en");
        /* store this modified value in the output */
        line = g_strdup_printf("<!ENTITY %s \"%s\">", key, value);
        tmp = output_contents;
        output_contents = g_strjoin ("", output_contents, line, NULL);
        g_free (tmp);
        g_free (line);
        g_match_info_next (match_info, NULL);
        g_free (key);
    }
    /* free regex variables */
    g_match_info_free(match_info);
    g_regex_unref(string_regex);

    return output_contents;
}