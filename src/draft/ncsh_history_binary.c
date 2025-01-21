/*eskilib_nodiscard enum eskilib_Result history_write_entry_to_file(struct eskilib_String* entry, FILE* file)
{
    assert(file);

    size_t bytes_written;

    bytes_written = fwrite(&entry->length, sizeof(size_t), 1, file);
    if (bytes_written == 0 || feof(file))
        return E_FAILURE;
    else if (ferror(file))
        return E_FAILURE_FILE_OP;

    bytes_written = fwrite(entry->value, sizeof(char), entry->length, file);
    if (bytes_written == 0)
        return E_FAILURE;
    else if (ferror(file))
        return E_FAILURE_FILE_OP;

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_save_v2(struct ncsh_History* history)
{
    assert(history);
    if (!history)
        return E_FAILURE_NULL_REFERENCE;
    if (history->count == 0)
        return E_SUCCESS;

    FILE* file = fopen(history->history_file, "wb");
    if (!file || feof(file) || ferror(file)) {
        perror("Error writing to history file");
        if (file)
            fclose(file);
        return E_FAILURE_FILE_OP;
    }

    if (fwrite(&history->count, sizeof(uint32_t), 1, file) == 0 || feof(file) || ferror(file)) {
        perror("Error writing number of entries to history file, could not write to file");
        fclose(file);
        return E_FAILURE_FILE_OP;
    }

    enum eskilib_Result result;
    for (uint_fast32_t i = 0; i < history->count; ++i) {
        if ((result = history_write_entry_to_file((history->entries + i), file)) != E_SUCCESS) {
            fclose(file);
            return result;
        }
    }

    fclose(file);
    return E_SUCCESS;
}*/


