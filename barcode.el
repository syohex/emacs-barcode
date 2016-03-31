;;; barcode.el --- libbarcode binding

;; Copyright (C) 2016 by Syohei YOSHIDA

;; Author: Syohei YOSHIDA <syohex@gmail.com>
;; URL: https://github.com/syohex/emacs-barcode
;; Version: 0.01

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(require 'cl-lib)
(require 'image-mode)
(require 'barcode-core)

;;;###autoload
(cl-defun barcode-encode (str &optional (encoding 'code128))
  (interactive
   (list (if (use-region-p)
             (buffer-substring-no-properties (region-beginning) (region-end))
           (read-string "Input: "))))
  (let ((eps (barcode-core-encode str encoding)))
    (unless (stringp eps)
      (error "Error: converting '%s' to barcode!!" str))
    (with-current-buffer (get-buffer-create "*barcode*")
      (read-only-mode -1)
      (when (image-get-display-property)
        (image-toggle-display))
      (erase-buffer)
      (insert eps)
      (goto-char (point-min))
      (image-mode)
      (pop-to-buffer (current-buffer)))))

(provide 'barcode)

;;; barcode.el ends here
