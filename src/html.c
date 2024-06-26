typedef struct
{
	FILE*			f;
	const document*	doc;
	int				depth;
	int				ref_count;
	int				chapter_index;
	int				inline_ref_count;
	int				chapter_ref_count;
	int				inline_chapter_ref_count;
} html_context;

static void print_tabs(html_context* ctx, int depth)
{
	fputc('\n', ctx->f);

	for (int i = 0; i < depth; ++i)
		fputc('\t', ctx->f);
}

static void print_text_simple(html_context* ctx, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_en_dash)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x93, ctx->f);
		}
		else if (*text == text_token_type_em_dash)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x94, ctx->f);
		}
		else if (*text == '\'')
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x99, ctx->f);
		}
		else if (*text == text_token_type_quote_level_1_begin)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x9C, ctx->f);
		}
		else if (*text == text_token_type_quote_level_1_end)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x9D, ctx->f);
		}
		else if (*text >= ' ')
		{
			fputc(*text, ctx->f);
		}

		++text;
	}
}


static void print_text_block(html_context* ctx, const char* text)
{
	while (*text)
	{
		if (*text == text_token_type_strong_begin)
		{
			fputs("<strong>", ctx->f);
		}
		else if (*text == text_token_type_strong_end)
		{
			fputs("</strong>", ctx->f);
		}
		if (*text == text_token_type_emphasis_begin)
		{
			fputs("<em>", ctx->f);
		}
		else if (*text == text_token_type_emphasis_end)
		{
			fputs("</em>", ctx->f);
		}
		else if (*text == text_token_type_en_dash)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x93, ctx->f);
		}
		else if (*text == text_token_type_em_dash)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x94, ctx->f);
		}
		else if (*text == '\'')
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x99, ctx->f);
		}
		else if (*text == text_token_type_quote_level_1_begin)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x9C, ctx->f);
		}
		else if (*text == text_token_type_quote_level_1_end)
		{
			fputc(0xE2, ctx->f);
			fputc(0x80, ctx->f);
			fputc(0x9D, ctx->f);
		}
		else if (*text == text_token_type_reference)
		{
			const uint32_t ref_count = ctx->inline_ref_count++;
			const uint32_t chapter_ref_count = ctx->inline_chapter_ref_count++;

			const document_chapter* chapter = &ctx->doc->chapters[ctx->chapter_index];
			const document_reference* reference = &chapter->references[chapter_ref_count];

			fprintf(ctx->f, "<sup><a href=\"#r%d\" title=\"", ref_count + 1);
			print_text_simple(ctx, reference->text);
			fprintf(ctx->f, "\">[%d]</a></sup>", chapter_ref_count + 1);
		}
		else
		{
			fputc(*text, ctx->f);
		}

		++text;
	}
}

static void generate_html(const document* doc)
{
	const char* filepath = "out.html";

	FILE* f = fopen(filepath, "w");
	if (!f)
	{
		fprintf(stderr, "Error opening file for write '%s': %s.\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	html_context ctx = {
		.f		= f,
		.doc	= doc,
		.depth	= 2
	};

	fprintf(f,
		"<!DOCTYPE html>\n"
		"<html lang=\"en-GB\">\n"
		"\t<head>\n"
		"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
		"\t\t<link href=\"style.css\" rel=\"stylesheet\">\n"
		"\t</head>\n"
		"\t<body>"
	);

	if (doc->chapter_count > 1)
	{
		fprintf(f,
			"\n\t\t<h1>Table of Contents</h1>\n"
			"\t\t<p>\n"
			"\t\t\t<ul>\n"
		);

		for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
		{
			document_element* heading = doc->chapters[chapter_index].elements;

			fprintf(f, "\t\t\t\t<li><a href=\"#h%d\">", chapter_index + 1);
			print_text_simple(&ctx, heading->text);
			fprintf(f, "</a></li>\n");
		}

		fprintf(f,
			"\t\t\t</ul>\n"
			"\t\t</p>"
		);

		fprintf(f, "\n\t\t<hr>");
	}

	for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
	{
		document_chapter* chapter = &doc->chapters[chapter_index];
		ctx.chapter_index = chapter_index;

		for (uint32_t element_index = 0; element_index < chapter->element_count; ++element_index)
		{
			document_element* element = &chapter->elements[element_index];

			switch (element->type)
			{
			case document_element_type_heading_1:
				ctx.chapter_ref_count = 0;
				ctx.inline_chapter_ref_count = 0;

				print_tabs(&ctx, ctx.depth);
				fprintf(f, "<h1 id=\"h%d\">", chapter_index + 1);
				print_text_block(&ctx, element->text);
				fprintf(f, "</h1>");
				break;
			case document_element_type_heading_2:
				print_tabs(&ctx, ctx.depth);
				fprintf(f, "<h2>%s</h2>", element->text);
				break;
			case document_element_type_heading_3:
				print_tabs(&ctx, ctx.depth);
				fprintf(f, "<h3>%s</h3>", element->text);
				break;
			case document_element_type_text_block:
				print_tabs(&ctx, ctx.depth + 1);
				print_text_block(&ctx, element->text);
				break;
			case document_element_type_line_break:
				print_tabs(&ctx, ctx.depth + 1);
				fputs("<br>", f);
				break;
			case document_element_type_paragraph_begin:
				print_tabs(&ctx, ctx.depth);
				fputs("<p>", f);
				break;
			case document_element_type_paragraph_end:
				print_tabs(&ctx, ctx.depth);
				fputs("</p>", f);
				break;
			case document_element_type_blockquote_begin:
				print_tabs(&ctx, ctx.depth++);
				fputs("<blockquote>", f);
				break;
			case document_element_type_blockquote_end:
				print_tabs(&ctx, --ctx.depth);
				fputs("</blockquote>", f);
				break;
			case document_element_type_ordered_list_begin_roman:
				print_tabs(&ctx, ctx.depth++);
				fputs("<ol type=\"I\">", f);
				break;
			case document_element_type_ordered_list_begin_arabic:
				print_tabs(&ctx, ctx.depth++);
				fputs("<ol>", f);
				break;
			case document_element_type_ordered_list_begin_letter:
				print_tabs(&ctx, ctx.depth++);
				fputs("<ol type=\"a\">", f);
				break;
			case document_element_type_ordered_list_end:
				print_tabs(&ctx, --ctx.depth);
				fputs("</ol>", f);
				break;
			case document_element_type_ordered_list_item:
				print_tabs(&ctx, ctx.depth);
				fprintf(f, "<li>%s</li>", element->text);
				break;
			}
		}

		if (chapter_index != doc->chapter_count - 1)
			fprintf(f, "\n\t\t<hr>");

		if (chapter->reference_count > 0)
		{
			if (chapter_index == doc->chapter_count - 1)
				fprintf(f, "\n\t\t<hr>");

			for (uint32_t reference_index = 0; reference_index < chapter->reference_count; ++reference_index)
			{
				document_reference* reference = &chapter->references[reference_index];
				fprintf(f, "\n\t\t<p id=\"r%d\">\n", ++ctx.ref_count);
				fprintf(f, "\t\t\t[%d] ", ++ctx.chapter_ref_count);
				print_text_block(&ctx, reference->text);
				fprintf(f, "\n\t\t</p>");
			}

			if (chapter_index != doc->chapter_count - 1)
				fprintf(f, "\n\t\t<hr>");
		}
	}

	fprintf(f,
		"\n\t</body>\n"
		"</html>"
	);

	fclose(f);
}