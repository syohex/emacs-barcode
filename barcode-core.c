/*
  Copyright (C) 2016 by Syohei YOSHIDA

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _BSD_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <emacs-module.h>

#include <barcode.h>

int plugin_is_GPL_compatible;

static int
encoding_type(emacs_env *env, emacs_value type)
{
	if (env->eq(env, type, env->intern(env, "any"))) {
		return BARCODE_ANY;
	} else if (env->eq(env, type, env->intern(env, "ean"))) {
		return BARCODE_EAN;
	} else if (env->eq(env, type, env->intern(env, "upc"))) {
		return BARCODE_UPC;
	} else if (env->eq(env, type, env->intern(env, "isbn"))) {
		return BARCODE_ISBN;
	} else if (env->eq(env, type, env->intern(env, "code39"))) {
		return BARCODE_39;
	} else if (env->eq(env, type, env->intern(env, "code128"))) {
		return BARCODE_128;
	} else if (env->eq(env, type, env->intern(env, "code128c"))) {
		return BARCODE_128C;
	} else if (env->eq(env, type, env->intern(env, "code128b"))) {
		return BARCODE_128B;
	} else if (env->eq(env, type, env->intern(env, "i25"))) {
		return BARCODE_I25;
	} else if (env->eq(env, type, env->intern(env, "code128raw"))) {
		return BARCODE_128RAW;
	} else if (env->eq(env, type, env->intern(env, "CBR"))) {
		return BARCODE_128RAW;
	} else if (env->eq(env, type, env->intern(env, "msi"))) {
		return BARCODE_MSI;
	} else if (env->eq(env, type, env->intern(env, "pls"))) {
		return BARCODE_PLS;
	} else if (env->eq(env, type, env->intern(env, "code93"))) {
		return BARCODE_93;
	} else {
		return -1;
	}
}

static char*
retrieve_string(emacs_env *env, emacs_value str, ptrdiff_t *size)
{
	*size = 0;

	env->copy_string_contents(env, str, NULL, size);
	char *p = malloc(*size);
	if (p == NULL) {
		*size = 0;
		return NULL;
	}
	env->copy_string_contents(env, str, p, size);

	return p;
}

// fp should be set end of file and open with read-write mode.
static emacs_value
read_eps(emacs_env *env, FILE *fp)
{
	emacs_value Qnil = env->intern(env, "nil");
	long eps_size = ftell(fp);

	char *buf = (char*)malloc((size_t)(eps_size+1));
	if (buf == NULL) {
		return Qnil;
	}

	fseek(fp, 0, SEEK_SET);

	size_t len = fread(buf, eps_size, 1, fp);
	if (len != 1) {
		return Qnil;
	}

	emacs_value eret = env->make_string(env, buf, (intmax_t)eps_size);
	free(buf);

	return eret;
}

static emacs_value
Fbarcode_encode(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	emacs_value input = args[0], eret = env->intern(env, "nil");
	ptrdiff_t size;
	int ret, tmp_eps;
	struct Barcode_Item *bcode;
	FILE *fp;
	char *input_str = retrieve_string(env, input, &size);
	char tmpbuf[128];
	int encoding;

	encoding = encoding_type(env, args[1]);
	if (encoding == -1) {
		return eret;
	}

	bcode = Barcode_Create(input_str);
	if (bcode == NULL) {
		free(input_str);
		return eret;
	}

	memcpy(tmpbuf, "XXXXXX.eps", sizeof("XXXXXX.eps"));

	tmp_eps = mkstemps(tmpbuf, 4);
	if (tmp_eps == -1) {
		goto free;
	}

	ret = Barcode_Encode(bcode, encoding);
	if (ret < 0) {
		goto free;
	}

	fp = fdopen(tmp_eps, "r+");
	if (fp == NULL) {
		goto free;
	}

	ret = Barcode_Print(bcode, fp, BARCODE_OUT_EPS);
	if (ret < 0) {
		goto free;
	}

	eret = read_eps(env, fp);

	if (fclose(fp) != 0) {
		goto free;
	}

free:
	unlink(tmpbuf);
	free(input_str);
	Barcode_Delete(bcode);

	return eret;
}

static void
bind_function(emacs_env *env, const char *name, emacs_value Sfun)
{
	emacs_value Qfset = env->intern(env, "fset");
	emacs_value Qsym = env->intern(env, name);
	emacs_value args[] = { Qsym, Sfun };

	env->funcall(env, Qfset, 2, args);
}

static void
provide(emacs_env *env, const char *feature)
{
	emacs_value Qfeat = env->intern(env, feature);
	emacs_value Qprovide = env->intern (env, "provide");
	emacs_value args[] = { Qfeat };

	env->funcall(env, Qprovide, 1, args);
}

int
emacs_module_init(struct emacs_runtime *ert)
{
	emacs_env *env = ert->get_environment(ert);

#define DEFUN(lsym, csym, amin, amax, doc, data) \
	bind_function (env, lsym, env->make_function(env, amin, amax, csym, doc, data))

	DEFUN("barcode-core-encode", Fbarcode_encode, 2, 2, NULL, NULL);

#undef DEFUN

	provide(env, "barcode-core");
	return 0;
}

/*
  Local Variables:
  c-basic-offset: 8
  indent-tabs-mode: t
  End:
*/
