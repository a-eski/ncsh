
/*int stmt_next(Parser_Data* restrict data, enum Logic_Type type)
{
    if (!data->cur_stmt->prev) {
        data->cur_stmt->type = type;
        data->cur_stmt->right = stmt_alloc(data->s);
        data->cur_stmt->right->prev = data->cur_stmt;
        data->cur_stmt = data->cur_stmt->right;
        return EXIT_SUCCESS;
    }

    switch (data->cur_stmt->prev->type) {
    case LT_IF_CONDITIONS: {
        switch (type) {
        case LT_IF:
            goto right;
        case LT_ELSE:
            goto left;
        case LT_ELIF_CONDITIONS:
        case LT_NORMAL:
        case LT_IF_CONDITIONS:
        case LT_ELIF:
            return EXIT_FAILURE_CONTINUE;
        }
        break;
    }

    case LT_IF: {
        switch (type) {
        case LT_NORMAL:
        case LT_IF:
            goto right;
        case LT_ELSE:
            goto left;
        case LT_IF_CONDITIONS:
        case LT_ELIF_CONDITIONS:
        case LT_ELIF:
            return EXIT_FAILURE_CONTINUE;
        }
        break;
    }

    case LT_ELSE: {
        switch (type) {
        case LT_NORMAL:
        case LT_ELSE:
            goto right;
        case LT_IF:
        case LT_IF_CONDITIONS:
        case LT_ELIF_CONDITIONS:
        case LT_ELIF:
            return EXIT_FAILURE_CONTINUE;
        }
        break;
    }

    case LT_NORMAL: {
        switch (type) {
        case LT_IF_CONDITIONS:
        case LT_NORMAL:
            goto right;
        case LT_ELSE:
        case LT_IF:
        case LT_ELIF_CONDITIONS:
        case LT_ELIF:
            return EXIT_FAILURE_CONTINUE;
        }
        break;
    }

    case LT_ELIF_CONDITIONS: {
        switch (type) {
        case LT_ELIF:
            goto right;
        case LT_ELSE:
        case LT_NORMAL:
        case LT_IF:
        case LT_ELIF_CONDITIONS:
        case LT_IF_CONDITIONS: {
            return EXIT_FAILURE_CONTINUE;
        }
        }
        break;
    }

    case LT_ELIF: {
        switch (type) {
        case LT_ELSE:
            goto left;
        case LT_ELIF:
        case LT_NORMAL:
        case LT_IF:
        case LT_ELIF_CONDITIONS:
        case LT_IF_CONDITIONS: {
            return EXIT_FAILURE_CONTINUE;
        }
        }
        break;
    }
    }

    return EXIT_FAILURE_CONTINUE;

right:
    data->cur_stmt->type = type;
    data->cur_stmt->right = stmt_alloc(data->s);
    data->cur_stmt->right->prev = data->cur_stmt;
    data->cur_stmt = data->cur_stmt->right;
    return EXIT_SUCCESS;

left:
    if (type == LT_ELSE) {
        Statement* stmt = data->cur_stmt;

        do {
            if (!stmt)
                return EXIT_FAILURE_CONTINUE;
            stmt = stmt->prev;
        } while (stmt && stmt->prev && stmt->type != LT_IF_CONDITIONS);

        if (!stmt)
            return EXIT_FAILURE_CONTINUE;

        stmt->left = stmt_alloc(data->s);
        stmt->left->prev = stmt;
        data->cur_stmt = stmt->left;
        goto right;
    }
    else {
        data->cur_stmt->prev->left = stmt_alloc(data->s);
        data->cur_stmt->prev->left->prev = data->cur_stmt;
        data->cur_stmt = data->cur_stmt->prev->left;
        goto right;
    }

    return EXIT_SUCCESS;
}*/

/*int stmt_next(Parser_Data* restrict data, enum Logic_Type type)
{
    if (!data->cur_stmt) {
        data->cur_stmt = stmt_alloc(data->s);
        data->cur_stmt->type = type;
        return EXIT_SUCCESS;
    }

    switch (type) {
    case LT_NORMAL:
    case LT_IF_CONDITIONS:
    case LT_IF:
    case LT_ELIF_CONDITIONS:
        goto right;
    case LT_ELIF:
    case LT_ELSE:
        goto left;
    }

    unreachable();
    return EXIT_FAILURE_CONTINUE;

right:
    data->cur_stmt->right = stmt_alloc(data->s);
    data->cur_stmt->right->prev = data->cur_stmt;
    data->cur_stmt = data->cur_stmt->right;
    data->cur_stmt->type = type;
    return EXIT_SUCCESS;

left:
    if (!data->cur_stmt->prev) {
        return EXIT_FAILURE_CONTINUE;
    }
    data->cur_stmt->prev->left = stmt_alloc(data->s);
    data->cur_stmt->prev->left->prev = data->cur_stmt;
    data->cur_stmt = data->cur_stmt->prev->left;
    data->cur_stmt->type = type;
    return EXIT_SUCCESS;
}*/
