#include <vellum/shell.h>

#include <stdint.h>

struct shell_var_slot {
    struct shell_var var;
    char storage[126];
};

#define SHELL_VAR_CAPACITY 16

static struct shell_var_slot shell_var_pool[SHELL_VAR_CAPACITY];
static uint8_t shell_var_used[SHELL_VAR_CAPACITY];

static int key_matches(const struct shell_var *entry, const char *key)
{
    size_t i;

    if (!entry || !key) return 0;

    for (i = 0; i < entry->key_len; ++i) {
        if (entry->str[i] != key[i]) return 0;
    }

    return key[entry->key_len] == '\0';
}

static struct shell_var_slot *find_slot(struct shell_var *entry)
{
    size_t i;

    for (i = 0; i < SHELL_VAR_CAPACITY; ++i) {
        if (&shell_var_pool[i].var == entry) {
            return &shell_var_pool[i];
        }
    }

    return NULL;
}

static struct shell_var_slot *allocate_slot(void)
{
    size_t i;

    for (i = 0; i < SHELL_VAR_CAPACITY; ++i) {
        if (!shell_var_used[i]) {
            shell_var_used[i] = 1;
            shell_var_pool[i] = (struct shell_var_slot){0};
            return &shell_var_pool[i];
        }
    }

    return NULL;
}

status_t VlShell_GetVariable(struct shell_instance *inst, const char *key, const char **value)
{
    struct shell_var *current;

    if (!inst || !key) return STATUS_INVALID_VALUE;

    for (current = inst->var_list; current; current = current->next) {
        if (key_matches(current, key)) {
            if (value) *value = &current->str[current->key_len + 1];
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ENTRY_NOT_FOUND;
}

status_t VlShell_SetVariable(struct shell_instance *inst, const char *key, const char *value)
{
    struct shell_var *current;
    struct shell_var_slot *slot = NULL;
    size_t key_len = 0;
    size_t value_len = 0;
    size_t i;

    if (!inst || !key || !value) return STATUS_INVALID_VALUE;

    while (key[key_len]) {
        ++key_len;
    }
    while (value[value_len]) {
        ++value_len;
    }

    if (key_len + value_len + 2 > sizeof(shell_var_pool[0].storage)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    for (current = inst->var_list; current; current = current->next) {
        if (key_matches(current, key)) {
            slot = find_slot(current);
            break;
        }
    }

    if (!slot) {
        slot = allocate_slot();
        if (!slot) return STATUS_INSUFFICIENT_MEMORY;

        slot->var.next = inst->var_list;
        inst->var_list = &slot->var;
    }

    slot->var.key_len = key_len;
    slot->var.value_len = value_len;

    for (i = 0; i < key_len; ++i) {
        slot->var.str[i] = key[i];
    }
    slot->var.str[key_len] = '\0';

    for (i = 0; i < value_len; ++i) {
        slot->var.str[key_len + 1 + i] = value[i];
    }
    slot->var.str[key_len + 1 + value_len] = '\0';

    return STATUS_SUCCESS;
}

status_t VlShell_RemoveVariable(struct shell_instance *inst, const char *key)
{
    struct shell_var *prev = NULL;
    struct shell_var *current;
    struct shell_var_slot *slot;

    if (!inst || !key) return STATUS_INVALID_VALUE;

    for (current = inst->var_list; current; current = current->next) {
        if (!key_matches(current, key)) {
            prev = current;
            continue;
        }

        if (prev) {
            prev->next = current->next;
        } else {
            inst->var_list = current->next;
        }

        slot = find_slot(current);
        if (slot) {
            size_t i;

            for (i = 0; i < SHELL_VAR_CAPACITY; ++i) {
                if (&shell_var_pool[i] == slot) {
                    shell_var_used[i] = 0;
                    shell_var_pool[i] = (struct shell_var_slot){0};
                    break;
                }
            }
        }

        return STATUS_SUCCESS;
    }

    return STATUS_ENTRY_NOT_FOUND;
}
