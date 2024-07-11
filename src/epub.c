static void create_epub_mimetype(void)
{
	FILE* f = open_file("ebook_out/mimetype", file_mode_write);

	fputs("application/epub+zip", f);

	fclose(f);
}

static void create_epub_meta_inf(void)
{
	FILE* f = open_file("ebook_out/META-INF/container.xml", file_mode_write);

	fputs(
		"<?xml version=\"1.0\"?>\n"
		"<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
		"\t<rootfiles>\n"
		"\t\t<rootfile full-path=\"content.opf\" media-type=\"application/oebps-package+xml\" />\n"
		"\t</rootfiles>\n"
		"</container>",
		f
	);

	fclose(f);
}

static void create_epub_css(void)
{
	FILE* f = open_file("ebook_out/style.css", file_mode_write);

	fputs(
		"",
		f
	);

	fclose(f);
}

static void create_epub_opf(const document* doc)
{
	FILE* f = open_file("ebook_out/content.opf", file_mode_write);

	fputs(
		"<?xml version=\"1.0\"?>\n"
		"<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"bookid\">\n"
		"\t<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n",
		f
	);

	fprintf(f, "\t\t<dc:title>%s</dc:title>\n", doc->metadata.title);
	fprintf(f, "\t\t<dc:language>en-GB</dc:language>\n");
	fprintf(f, "\t\t<dc:identifier id=\"bookid\" opf:scheme=\"uuid\">6519aff0-f47c-437c-ba20-cc0782b39b05</dc:identifier>\n");

	for (uint32_t i = 0; i < doc->metadata.author_count; ++i)
		fprintf(f, "\t\t<dc:creator>%s</dc:creator>\n", doc->metadata.authors[i]);

//	for (uint32_t i = 0; i < doc->metadata.translator_count; ++i)
//		fprintf(f, "\t\t<dc:creator opf:role=\"translator\">%s</dc:creator>\n", doc->metadata.translators[i]);

	fputs("\t\t<meta name=\"cover\" content=\"cover_image\"/>\n", f);

	fputs("\t</metadata>\n", f);
	fputs("\t<manifest>\n", f);
	fputs("\t\t<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n", f);
	fputs("\t\t<item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n", f);

	fprintf(f, "\t\t<item id=\"cover\" href=\"cover.xhtml\" media-type=\"application/xhtml+xml\"/>\n");
	fprintf(f, "\t\t<item id=\"cover_image\" href=\"cover.png\" media-type=\"image/png\"/>\n");

	if (doc->chapter_count > 0)
		fprintf(f, "\t\t<item id=\"toc\" href=\"toc.xhtml\" media-type=\"application/xhtml+xml\"/>\n");

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		fprintf(f, "\t\t<item id=\"chapter%d\" href=\"chapter%d.xhtml\" media-type=\"application/xhtml+xml\"/>\n", i + 1, i + 1);
	fputs("\t</manifest>\n", f);

	fputs("\t<spine toc=\"ncx\">\n", f);

	fputs("\t\t<itemref idref=\"cover\"/>\n", f);

	if (doc->chapter_count > 0)
		fputs("\t\t<itemref idref=\"toc\"/>\n", f);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		fprintf(f, "\t\t<itemref idref=\"chapter%d\"/>\n", i + 1);

	fputs("\t</spine>\n", f);

	fputs("</package>", f);

	fclose(f);
}

static void create_epub_ncx(const document* doc)
{
	FILE* f = open_file("ebook_out/toc.ncx", file_mode_write);

	fputs(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
		"\t<head>\n"
		"\t\t<meta name=\"dtb:uid\" content=\"6519aff0-f47c-437c-ba20-cc0782b39b05\"/>\n"
//		"\t\t<meta name=\"dtb:depth\" content=\"1\"/>\n"
//		"\t\t<meta name=\"dtb:totalPageCount\" content=\"0\"/>\n"
//		"\t\t<meta name=\"dtb:maxPageNumber\" content=\"0\"/>\n"
		"\t</head>\n",
		f
	);

	fputs("\t<docTitle>", f);
	fprintf(f, "\t\t<text>%s</text>\n", doc->metadata.title);
	fputs("\t</docTitle>", f);

	fputs("\t<navMap>\n", f);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
	{
		document_element* heading = doc->chapters[i].elements;

		fprintf(f, "\t\t<navPoint class=\"chapter\" id=\"chapter%d\" playOrder=\"%d\">\n", i + 1, i + 1);
		fputs("\t\t\t<navLabel>\n", f);
		fprintf(f, "\t\t\t\t<text>%s</text>\n", heading->text);
		fputs("\t\t\t</navLabel>\n", f);
		fprintf(f, "\t\t\t<content src=\"chapter%d.xhtml\"/>\n", i + 1);
		fputs("\t\t</navPoint>\n", f);
	}

	fputs("\t</navMap>\n", f);
	fputs("</ncx>", f);

	fclose(f);
}

static void create_epub_toc(const document* doc)
{
	if (doc->chapter_count == 0)
		return;

	FILE* f = open_file("ebook_out/toc.xhtml", file_mode_write);

	html_context ctx = {
		.f		= f,
		.doc	= doc,
		.depth	= 2
	};

	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"\t<head>\n"
		"\t\t<title>Table of Contents</title>\n"
		//"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<link href=\"style.css\" rel=\"stylesheet\">\n"
		"\t</head>\n"
		"\t<body>\n"
		"\t\t<h1>Table of Contents</h1>\n"
		"\t\t<p>\n"
		"\t\t\t<ul>"
	);

	for (uint32_t chapter_index = 0; chapter_index < doc->chapter_count; ++chapter_index)
	{
		document_element* heading = doc->chapters[chapter_index].elements;

		fprintf(f, "\t\t\t\t<li><a href=\"chapter%d.xhtml\">", chapter_index + 1);
		print_text_simple(&ctx, heading->text);
		fprintf(f, "</a></li>\n");
	}

	fprintf(f,
		"\n\t\t</ul>\n"
		"\n\t</body>\n"
		"</html>"
	);

	fclose(f);
}

static void create_epub_chapter(const document* doc, uint32_t index)
{
	// TODO: Check path sizes
	char path[256];
	const int len = snprintf(path, sizeof(path), "ebook_out/chapter%d.xhtml", index + 1);
	assert(len < sizeof(path));

	FILE* f = open_file(path, file_mode_write);

	html_context ctx = {
		.f				= f,
		.doc			= doc,
		.depth			= 2,
		.chapter_index	= index
	};

	document_chapter* chapter = &doc->chapters[index];

	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"\t<head>\n"
		"\t\t<title>%s</title>\n"
		//"\t\t<meta charset=\"UTF-8\">\n"
		"\t\t<link href=\"style.css\" rel=\"stylesheet\">\n"
		"\t</head>\n"
		"\t<body>",
		chapter->elements[0].text
	);

	for (uint32_t element_index = 0; element_index < chapter->element_count; ++element_index)
	{
		document_element* element = &chapter->elements[element_index];

		switch (element->type)
		{
		case document_element_type_heading_1:
			ctx.chapter_ref_count = 0;
			ctx.inline_chapter_ref_count = 0;

			print_tabs(&ctx, ctx.depth);
			fprintf(f, "<h1>");
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
			fputs("<br/>", f);
			break;
		case document_element_type_paragraph_begin:
			print_tabs(&ctx, ctx.depth);
			fputs("<p>", f);
			break;
		case document_element_type_paragraph_break_begin:
			print_tabs(&ctx, ctx.depth);
			fputs("<p class=\"paragraph-break\">", f);
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

	if (chapter->reference_count > 0)
	{
		for (uint32_t reference_index = 0; reference_index < chapter->reference_count; ++reference_index)
		{
			++ctx.ref_count;
			++ctx.chapter_ref_count;

			document_reference* reference = &chapter->references[reference_index];
			fprintf(f, "\n\t\t<p class=\"footnote\" id=\"ref%d\">\n", ctx.ref_count);
			fprintf(f, "\t\t\t[<a href=\"#ref-return%d\">%d</a>] ", ctx.ref_count, ctx.chapter_ref_count);
			print_text_block(&ctx, reference->text);
			fprintf(f, "\n\t\t</p>");
		}
	}

	fprintf(f,
		"\n\t</body>\n"
		"</html>"
	);

	fclose(f);
}

static void generate_epub(const document* doc)
{
	create_epub_mimetype();
	create_epub_meta_inf();
	//create_epub_css();
	create_epub_opf(doc);
	create_epub_ncx(doc);
	create_epub_toc(doc);

	for (uint32_t i = 0; i < doc->chapter_count; ++i)
		create_epub_chapter(doc, i);
}