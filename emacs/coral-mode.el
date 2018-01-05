(defvar coral-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map [foo] 'coral-do-foo)
    map)
  "Keymap for `coral-mode'.")

(defvar coral-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?# "<" st)
    (modify-syntax-entry ?\n ">" st)
    (modify-syntax-entry ?\' "\"\'" st)
    (modify-syntax-entry ?\" "\"\"" st)
    st)
  "Syntax table for `coral-mode'.")

(defvar coral-font-lock-keywords '(
   ("^[ ]+\\<set\\>" . font-lock-keyword-face)
   ("\\<\\(impl\\|class\\)\\>" . font-lock-keyword-face)
   ("\\<\\(return\\|pass\\|if\\|then\\|for\\|in\\|func\\|let\\|type\\|match\\|with\\|module\\)\\>" . font-lock-keyword-face)
   ("=>\\|\[$@<>=+*/%-\]" . font-lock-function-name-face)
 ) "Keyword highlighting specification for `coral-mode'.")

(defvar coral-imenu-generic-expression '(
    ("Types" "type \\(\w+\\)" 1)
    ("Functions" "func \\(\w+\\)" 0)
    ))

(defvar coral-outline-regexp
  "\\<func\\|\\<extern\\|^#\\|\\<type")

 ;;;###autoload
(define-derived-mode coral-mode fundamental-mode "Coral"
  "A major mode for editing Coral files."
  :syntax-table coral-mode-syntax-table
  (setq-local comment-start "# ")
  (setq-local comment-start-skip "#+\\s-*")
  (setq-local font-lock-defaults
	      '(coral-font-lock-keywords))
  ;; (setq-local indent-line-function 'coral-indent-line)
  (setq-local imenu-generic-expression
	      coral-imenu-generic-expression)
  (setq-local outline-regexp coral-outline-regexp)
  (setq-local indent-tabs-mode nil)
  (outline-minor-mode 1)
  (add-to-list 'write-file-functions 'delete-trailing-whitespace)
  ())

 ;;; Indentation

;; (defun coral-indent-line ()
;;   "Indent current line of Coral code."
;;   (interactive)
;;   (let ((savep (> (current-column) (current-indentation)))
;; 	(indent (condition-case nil (max (coral-calculate-indentation) 0)
;; 		  (error 0))))
;;     (if savep
;; 	(save-excursion (indent-line-to indent))
;;       (indent-line-to indent))))

;; (defun coral-calculate-indentation ()
;;   "Return the column to which the current line should be indented."
;;   ())


(provide 'coral-mode)
(add-to-list 'auto-mode-alist '("\\.coral\\'" . coral-mode))
