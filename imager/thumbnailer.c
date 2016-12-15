#include "thumbnailer.h"
#include <string.h>

const unsigned long thumbWidth = 150, thumbHeight = 150;

int thumbnail(const void *src, const size_t size, const struct Options opts,
	      struct Thumbnail *thumb, ExceptionInfo *ex)
{
	ImageInfo *info = NULL;
	Image *img = NULL, *sampled = NULL, *scaled = NULL;
	double scale;
	int code = 0;

	// Read image
	info = CloneImageInfo(NULL);
	GetExceptionInfo(ex);
	img = BlobToImage(info, src, size, ex);
	if (img == NULL) {
		goto end;
	}
	thumb->srcWidth = img->columns;
	thumb->srcHeight = img->rows;

	// Validate dimentions
	if (strcmp(img->magick, "PDF")) {
		if (img->columns > opts.maxSrcWidth) {
			code = 2;
			goto end;
		}
		if (img->rows > opts.maxSrcHeight) {
			code = 3;
			goto end;
		}
	}

	// Image already fits thumbnail
	if (img->columns <= thumbWidth && img->rows <= thumbHeight) {
		thumb->width = img->columns;
		thumb->height = img->rows;
		writeThumb(img, thumb, opts, ex);
		goto end;
	}

	// Maintain aspect ratio
	if (img->columns >= img->rows) {
		scale = (double)(img->columns) / (double)(thumbWidth);
	} else {
		scale = (double)(img->rows) / (double)(thumbHeight);
	}
	thumb->width = (unsigned long)(img->columns / scale);
	thumb->height = (unsigned long)(img->rows / scale);

	// Subsample to 4 times the thumbnail size. A decent enough compromise
	// between quality and performance for images arround the thumbnail size
	// and much bigger ones.
	sampled = SampleImage(img, thumb->width * 4, thumb->height * 4, ex);
	if (sampled == NULL) {
		goto end;
	}

	// Scale to thumbnail size
	scaled =
	    ResizeImage(sampled, thumb->width, thumb->height, BoxFilter, 1, ex);
	if (scaled == NULL) {
		goto end;
	}

	writeThumb(scaled, thumb, opts, ex);

end:
	if (img != NULL) {
		DestroyImage(img);
	}
	if (info != NULL) {
		DestroyImageInfo(info);
	}
	if (sampled != NULL) {
		DestroyImage(sampled);
	}
	if (scaled != NULL) {
		DestroyImage(scaled);
	}
	if (code == 0) {
		return thumb->buf == NULL;
	}
	return code;
}

// Convert thumbnail to apropriate file type and write to buffer
static void writeThumb(Image *img, struct Thumbnail *thumb,
		       const struct Options opts, ExceptionInfo *ex)
{
	ImageInfo *info = CloneImageInfo(NULL);
	char *format = NULL;

	if (strcmp(img->magick, "JPEG")) {
		format = "PNG";
		// Will pass through libimagequant, so comression and filters
		// are pointeless
		info->quality = 0;
	} else {
		format = "JPEG";
		info->quality = opts.JPEGCompression;
	}
	strcpy(info->magick, format);
	strcpy(img->magick, format);
	thumb->buf = ImageToBlob(info, img, &thumb->size, ex);

	DestroyImageInfo(info);
}