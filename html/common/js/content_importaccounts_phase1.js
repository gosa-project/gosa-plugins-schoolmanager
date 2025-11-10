/**
 * Phase 1 CSV Upload - Generic Controllers
 *
 * This module is theme-agnostic and configuration-driven.
 * All element selectors, classes, strings, and settings are provided
 * via the configuration object passed to the constructor.
 *
 * @requires Configuration object with elements, classes, translations, settings, data
 * @see README-JS-FILES.md for configuration guidelines
 */

/**
 * FileUploadController - Main orchestrator for Phase 1 CSV upload
 *
 * Manages file upload, validation, CSV parsing, and coordinates with CSVTableController.
 *
 * @class
 */
class FileUploadController {
    /**
     * Creates a new FileUploadController instance
     * @param {Object} config - Configuration object from template
     * @param {Object} config.elements - DOM element IDs
     * @param {Object} config.classes - CSS class names
     * @param {Object} config.selectors - CSS selectors for querying
     * @param {Object} config.translations - Translated strings
     * @param {Object} config.settings - Business logic settings
     * @param {Object} config.data - Data structures (account types, delimiters, mappings)
     */
    constructor(config) {
        console.log('[FileUploadController] Initializing with config:', config);
        this._validateConfig(config);
        this.config = config;

        // Cache frequently accessed elements
        this.dropZone         = document.getElementById(this.config.elements.dropZone);
        this.dropZoneIcon     = document.getElementById(this.config.elements.dropZoneIcon);
        this.dropZoneTitle    = document.getElementById(this.config.elements.dropZoneTitle);
        this.dropZoneSubtitle = document.getElementById(this.config.elements.dropZoneSubtitle);
        this.delimiterWarning = document.getElementById(this.config.elements.delimiterWarning);
        this.errorMessage     = document.getElementById(this.config.elements.errorMessage);
        this.csvPreview       = document.getElementById(this.config.elements.csvPreview);
        this.fileInput        = document.getElementById(this.config.elements.fileInput);

        // State
        this.currentFile = null;
        this.currentCSVData = null;

        // Initialize sub-controller for table management
        this.tableController = null;

        this.init();
    }

    /**
     * Initialize the controller
     * @private
     */
    init() {
        console.log('[FileUploadController] Starting initialization');
        if (!this.dropZone || !this.fileInput) {
            console.error('[FileUploadController] Required elements not found');
            return;
        }

        this._initDragAndDrop();
        this._initCollapsible();
        this._initTemplateDownload();
        this._initFileInput();
        this._initDelimiterChange();
        this._initHeaderCheckbox();
        this._disableSubmitButton();
        console.log('[FileUploadController] Initialization complete');
    }

    /**
     * Initialize drag and drop functionality
     * @private
     */
    _initDragAndDrop() {
        // Click to browse
        this.dropZone.addEventListener('click', (e) => {
            if (e.target.tagName !== 'INPUT') {
                this.fileInput.click();
            }
        });

        // Drag events
        this.dropZone.addEventListener('dragleave', (e) => this._handleDragLeave(e));
        this.dropZone.addEventListener('dragover',  (e) => this._handleDragOver(e));
        this.dropZone.addEventListener('drop',      (e) => this._handleDrop(e));
    }

    /**
     * Handle drag leave event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragLeave(e) {
        e.preventDefault();
        e.stopPropagation();
        this.dropZone.classList.remove(this.config.classes.dragover);
    }

    /**
     * Handle drop event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDrop(e) {
        e.preventDefault();
        e.stopPropagation();
        this.dropZone.classList.remove(this.config.classes.dragover);

        const files = e.dataTransfer.files;
        if (files.length > 0) {
            this.fileInput.files = files;
            this._handleFileSelection(files[0]);
        }
    }

    /**
     * Initialize file input change handler
     * @private
     */
    _initFileInput() {
        this.fileInput.addEventListener('change', (e) => {
            if (e.target.files.length > 0) {
                this._handleFileSelection(e.target.files[0]);
            }
        });
    }

    /**
     * Handle file selection (from input or drag-drop)
     * @param {File} file - Selected file
     * @private
     */
    _handleFileSelection(file) {
        console.log('[FileUploadController] File selected:', file.name, 'Size:', file.size, 'bytes');

        // Clear previous errors and warnings
        this._hideError();
        this._hideWarning();
        this._disableSubmitButton();

        // Validate file type
        const ext = file.name.toLowerCase().split('.').pop();
        if (!this.config.settings.allowedExtensions.includes('.' + ext)) {
            console.warn('[FileUploadController] Invalid file type:', ext);
            this.showError(this.config.translations.invalidFileType);
            return;
        }

        // Validate file size
        if (file.size > this.config.settings.maxFileSize) {
            console.warn('[FileUploadController] File too large:', file.size, 'Max:', this.config.settings.maxFileSize);
            const sizeMB = (this.config.settings.maxFileSize / 1048576).toFixed(1);
            const errorMsg = this.config.translations.fileTooLarge.replace('%1', sizeMB);
            this.showError(errorMsg);
            return;
        }

        // Store current file
        this.currentFile = file;

        // Update drop zone to show file info
        this.dropZone.classList.add(this.config.classes.hasFile);
        this.dropZoneIcon.innerHTML = '<i class="fa ' + this.config.classes.fileUploadedFaIcon + '"></i>';
        this.dropZoneTitle.textContent = file.name;
        this.dropZoneSubtitle.textContent = this._formatFileSize(file.size);

        console.log('[FileUploadController] File validated, starting CSV parse');
        // Read and parse the CSV file
        this._readAndParseCSV(file);
    }

    /**
     * Read and parse CSV file
     * @param {File} file - File to read
     * @private
     */
    _readAndParseCSV(file) {
        console.log('[FileUploadController] Reading CSV file');
        const reader = new FileReader();
        reader.onload = (e) => this._parseAndDisplayCSV(e.target.result);
        reader.onerror = () => {
            console.error('[FileUploadController] Error reading file');
            this.showError(this.config.translations.readError);
        };
        reader.readAsText(file);
    }

    /**
     * Parse and display CSV content
     * @param {string} content - CSV file content
     * @private
     */
    _parseAndDisplayCSV(content) {
        console.log('[FileUploadController] Parsing CSV content, length:', content.length);
        const delimiter = this._getSelectedDelimiter();
        const hasHeaders = this._getHasHeaders();
        console.log('[FileUploadController] Using delimiter:', JSON.stringify(delimiter), 'Has headers:', hasHeaders);

        // Split into lines and parse
        const lines = content.split(/\r?\n/);
        const parsedData = [];

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim();
            if (line.length === 0) continue;

            const row = this._parseCSVLine(line, delimiter);
            parsedData.push(row);
        }

        console.log('[FileUploadController] Parsed', parsedData.length, 'rows');

        if (parsedData.length === 0) {
            console.warn('[FileUploadController] No data parsed from CSV');
            this.showError(this.config.translations.emptyFile);
            return;
        }

        console.log('[FileUploadController] First row columns:', parsedData[0].length);

        // Store parsed data
        this.currentCSVData = parsedData;

        // Initialize table controller if not already done
        if (!this.tableController) {
            console.log('[FileUploadController] Initializing CSVTableController');
            this.tableController = new CSVTableController(this.config.table);
        }

        // Display the data
        this.tableController.displayData(parsedData, hasHeaders);

        // Show preview section
        this.csvPreview.classList.add(this.config.classes.active);

        // Check for delimiter warning
        this._checkDelimiterWarning(parsedData[0].length);

        // Enable submit button
        this._enableSubmitButton();
        console.log('[FileUploadController] CSV parsing and display complete');
    }

    /**
     * Parse a single CSV line respecting quotes
     * @param {string} line - CSV line to parse
     * @param {string} delimiter - Delimiter character
     * @returns {Array<string>} Parsed values
     * @private
     */
    _parseCSVLine(line, delimiter) {
        const result = [];
        let current = '';
        let inQuotes = false;

        for (let i = 0; i < line.length; i++) {
            const char = line[i];

            if (char === '"') {
                if (inQuotes && line[i + 1] === '"') {
                    // Escaped quote inside quoted string
                    current += '"';
                    i++;
                } else {
                    // Toggle quote state
                    inQuotes = !inQuotes;
                }
            } else if (char === delimiter && !inQuotes) {
                // End of field - trim only if not quoted
                result.push(current.trim());
                current = '';
            } else {
                current += char;
            }
        }

        // Push the last field
        result.push(current.trim());
        return result;
    }

    /**
     * Get currently selected delimiter
     * @returns {string} Delimiter character
     * @private
     */
    _getSelectedDelimiter() {
        const selected = document.querySelector(this.config.selectors.delimiterRadios + ':checked');
        if (!selected) {
            console.error('[FileUploadController] No delimiter radio button is selected!');
            throw new Error('No delimiter selected - this is a bug!');
        }

        // All radio buttons have data-delimiter attribute
        const delimiter = selected.dataset.delimiter;
        if (!delimiter) {
            console.error('[FileUploadController] Selected radio button has no data-delimiter attribute!');
            throw new Error('Delimiter attribute missing - this is a bug!');
        }

        return delimiter;
    }

    /**
     * Get whether first line has headers
     * @returns {boolean} True if headers checkbox is checked
     * @private
     */
    _getHasHeaders() {
        const checkbox = document.getElementById(this.config.elements.csvWithHeaders);
        return checkbox ? checkbox.checked : false;
    }

    /**
     * Check and show delimiter warning if only one column detected
     * @param {number} columnCount - Number of columns detected
     * @private
     */
    _checkDelimiterWarning(columnCount) {
        if (!this.delimiterWarning) return;

        if (columnCount === 1) {
            console.warn('[FileUploadController] Only one column detected, showing delimiter warning');
            this.delimiterWarning.classList.add(this.config.classes.active);
        } else {
            this.delimiterWarning.classList.remove(this.config.classes.active);
        }
    }

    /**
     * Initialize delimiter change handler
     * @private
     */
    _initDelimiterChange() {
        const delimiterRadios = document.querySelectorAll(this.config.selectors.delimiterRadios);
        delimiterRadios.forEach((radio) => {
            radio.addEventListener('change', () => {
                if (this.currentFile) {
                    console.log('[FileUploadController] Delimiter changed, re-parsing CSV');
                    this._readAndParseCSV(this.currentFile);
                }
            });
        });
    }

    /**
     * Initialize header checkbox handler
     * @private
     */
    _initHeaderCheckbox() {
        const checkbox = document.getElementById(this.config.elements.csvWithHeaders);
        if (!checkbox) return;

        checkbox.addEventListener('change', () => {
            if (this.currentCSVData) {
                const hasHeaders = checkbox.checked;
                this.tableController.displayData(this.currentCSVData, hasHeaders);
            }
        });
    }

    /**
     * Initialize collapsible sections
     * @private
     */
    _initCollapsible() {
        const collapsibleSections = document.querySelectorAll(this.config.selectors.collapsibleSections);

        collapsibleSections.forEach((section) => {
            const header = section.querySelector(this.config.selectors.collapsibleHeader);
            if (!header) return;

            header.addEventListener('click', () => {
                section.classList.toggle(this.config.classes.open);
            });
        });
    }

    /**
     * Initialize template download functionality
     * @private
     */
    _initTemplateDownload() {
        const downloadBtn = document.getElementById(this.config.elements.downloadTemplateBtn);
        if (!downloadBtn) return;

        downloadBtn.addEventListener('click', (e) => {
            e.preventDefault();
            this._generateAndDownloadTemplate();
        });
    }

    /**
     * Generate and download CSV template based on account type
     * @private
     */
    _generateAndDownloadTemplate() {
        console.log('[FileUploadController] Generating CSV template');
        const accountType = this.config.settings.accountType;
        const delimiter = this._getSelectedDelimiter();

        const csvContent = this._generateTemplateCSV(accountType, delimiter);

        console.log('[FileUploadController] Template generated, length:', csvContent.length);

        // Create blob and download
        const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
        const link = document.createElement('a');
        const url = URL.createObjectURL(blob);

        link.setAttribute('href', url);
        link.setAttribute('download', 'template_' + accountType + '.csv');
        link.style.visibility = 'hidden';
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
        console.log('[FileUploadController] Template download initiated');
    }

    /**
     * Generate template CSV content based on account type
     * @param {string} accountType - Account type
     * @param {string} delimiter - Delimiter character
     * @returns {string} CSV content
     * @private
     */
    _generateTemplateCSV(accountType, delimiter) {
        const templates = this.config.data.csvTemplates;
        if (!templates || !templates[accountType]) {
            console.warn('[FileUploadController] No template for account type: ' + accountType);
            return '';
        }

        const template = templates[accountType];
        const headers  = template.headers  || [];
        const examples = template.examples || [];

        console.log('[FileUploadController] Generating template for', accountType, 'with', headers.length, 'columns');

        // Build CSV content
        let csv = headers.join(delimiter) + '\n';
        examples.forEach((row) => {
            csv += row.join(delimiter) + '\n';
        });

        return csv;
    }

    /**
     * Show error message
     * @param {string} message - Error message to display
     * @public
     */
    showError(message) {
        if (!this.errorMessage) return;

        this.errorMessage.textContent = message;
        this.errorMessage.classList.add(this.config.classes.active);
    }

    /**
     * Hide error message
     * @private
     */
    _hideError() {
        if (!this.errorMessage) return;

        this.errorMessage.classList.remove(this.config.classes.active);
        this.errorMessage.textContent = '';
    }

    /**
     * Hide warning message
     * @private
     */
    _hideWarning() {
        if (!this.delimiterWarning) return;

        this.delimiterWarning.classList.remove(this.config.classes.active);
    }

    /**
     * Enable submit buttons when valid CSV is uploaded
     * @private
     */
    _enableSubmitButton() {
        const btnContinue = document.getElementById(this.config.elements.btnContinue);

        if (btnContinue) btnContinue.removeAttribute('disabled');
    }

    /**
     * Disable submit buttons
     * @private
     */
    _disableSubmitButton() {
        const btnContinue = document.getElementById(this.config.elements.btnContinue);

        if (btnContinue) btnContinue.setAttribute('disabled', 'disabled');
    }

    /**
     * Format file size for display
     * @param {number} bytes - File size in bytes
     * @returns {string} Formatted file size
     * @private
     */
    _formatFileSize(bytes) {
        if (bytes < 1024) {
            return bytes + ' bytes';
        } else if (bytes < 1048576) {
            return (bytes / 1024).toFixed(2) + ' KB';
        } else {
            return (bytes / 1048576).toFixed(2) + ' MB';
        }
    }

    /**
     * Validate configuration object
     * @param {Object} config - Configuration to validate
     * @throws {Error} If configuration is invalid
     * @private
     */
    _validateConfig(config) {
        if (!config) {
            throw new Error('[FileUploadController] Configuration object is required');
        }

        const required = ['elements', 'classes', 'selectors', 'translations', 'settings', 'data'];
        required.forEach((section) => {
            if (!config[section]) {
                throw new Error('[FileUploadController] config.' + section + ' is required');
            }
        });

        // Validate required elements
        const requiredElements = [
            'dropZone', 'fileInput', 'errorMessage', 'csvPreview'
        ];
        requiredElements.forEach((element) => {
            if (!config.elements[element]) {
                throw new Error('[FileUploadController] config.elements.' + element + ' is required');
            }
        });
    }
}

/**
 * CSVTableController - Manages CSV table display with column mapping and drag-drop
 *
 * Handles rendering of CSV data, column mapping dropdowns, and drag-drop column reordering.
 *
 * @class
 */
class CSVTableController {
    /**
     * Creates a new CSVTableController instance
     * @param {Object} config - Configuration object from template
     * @param {Object} config.elements - DOM element IDs
     * @param {Object} config.classes - CSS class names
     * @param {Object} config.translations - Translated strings
     * @param {Object} config.settings - Business logic settings
     * @param {Object} config.data - Data structures (mapping options, default mappings)
     */
    constructor(config) {
        console.log('[CSVTableController] Initializing with config:', config);
        this._validateConfig(config);
        this.config = config;

        // Cache elements
        this.csvTable     = document.getElementById(this.config.elements.csvTable);
        this.csvTableHead = document.getElementById(this.config.elements.csvTableHead);
        this.csvTableBody = document.getElementById(this.config.elements.csvTableBody);
        this.statRows     = document.getElementById(this.config.elements.statRows);
        this.statColumns  = document.getElementById(this.config.elements.statColumns);

        console.log('[CSVTableController] Elements cached, ready for data display');

        // State
        this.cdurrentData = null;
        this.hasHeaders = false;
        this.columnMappings = [];
        this.draggedColumn = null;

        // Hijack form submit button to inject form data
        const form = document.getElementById(this.config.elements.mainForm);
        if (form) {
            console.info('[CSVTableController] Binding to main form submit event…');
            form.addEventListener('submit', e => this._handleSubmit(e));
        } else {
            console.error('[CSVTableController] Main form element not found!');
        }
    }

    /**
     * Handle form submit to inject column mappings
     * @param {Event} e - Submit event
     * @private
     */
    _handleSubmit(e) {
        console.log('[CSVTableController] Handling form submit, injecting column mappings.');

        this._calcFormData(this.columnMappings);
    }

    /**
     * Inject hidden inputs for column mappings into form
     * @param {Array<string>} columnMappings - Current column mappings
     * @private
     */
    _calcFormData(columnMappings) {
        console.log('[CSVTableController] Injecting form data for column mappings');
        const form = document.getElementById(this.config.elements.mainForm);
        if (!form) {
            console.error('[CSVTableController] Main form element not found for injecting column mappings <input>s !');
            return;
        }

        const post_var_name = this.config.settings.postVarColumnMappings;

        // Remove existing hidden inputs if any
        const existingInputs = form.querySelectorAll(`input[name^="${post_var_name}"]`);
        existingInputs.forEach(input => form.removeChild(input));

        // Inject hidden inputs for column mappings
        columnMappings.forEach((mapping, index) => {
            const input = document.createElement('input');
            input.type = 'hidden';
            input.name = `${post_var_name}${index}`;
            input.value = mapping;
            form.appendChild(input);
        });

        // Add count object
        const countInput = document.createElement('input');
        countInput.type = 'hidden';
        countInput.name = `${post_var_name}count`;
        countInput.value = columnMappings.length;
        form.appendChild(countInput);
    }

    /**
     * Display CSV data in table
     * @param {Array<Array<string>>} data - CSV data rows
     * @param {boolean} hasHeaders - Whether first row is headers
     * @public
     */
    displayData(data, hasHeaders) {
        console.log('[CSVTableController] displayData called with', data?.length, 'rows, hasHeaders:', hasHeaders);

        if (!data || data.length === 0) {
            console.warn('[CSVTableController] No data to display');
            return;
        }

        this.currentData = data;
        this.hasHeaders = hasHeaders;

        // Calculate stats
        const startRow = hasHeaders ? 1 : 0;
        const rowCount = data.length - startRow;
        const colCount = data[0].length;

        console.log('[CSVTableController] Stats - Rows:', rowCount, 'Columns:', colCount);

        // Update stats
        if (this.statRows) this.statRows.textContent = rowCount;
        if (this.statColumns) this.statColumns.textContent = colCount;

        // Determine display columns (at least the expected count)
        const accountType = this.config.settings.accountType;
        const expectedColumns = this.config.data.defaultMappings.length || colCount;
        const displayColumns = Math.max(colCount, expectedColumns);

        console.log('[CSVTableController] Display columns:', displayColumns, '(expected:', expectedColumns, ')');

        // Initialize column mappings if needed
        if (this.columnMappings.length !== displayColumns) {
            this._initializeColumnMappings(displayColumns);
        }

        // Render table
        this._renderTableHeader(displayColumns);
        this._renderTableBody(data, displayColumns, hasHeaders);

        console.log('[CSVTableController] Table rendering complete');
    }

    /**
     * Initialize column mappings with defaults
     * @param {number} numColumns - Number of columns
     * @private
     */
    _initializeColumnMappings(numColumns) {
        console.log('[CSVTableController] Initializing column mappings for', numColumns, 'columns');

        const accountType = this.config.settings.accountType;
        const defaults = this.config.data.defaultMappings || [];

        this.columnMappings = [];
        for (let i = 0; i < numColumns; i++) {
            if (i < defaults.length) {
                this.columnMappings.push(defaults[i]);
            } else {
                // For extra columns beyond the default mapping
                const lastDefault = defaults[defaults.length - 1];
                this.columnMappings.push(lastDefault === 'course' ? 'course' : '---');
            }
        }

        console.info('[CSVTableController] Column mappings initialized:', this.columnMappings);
    }

    /**
     * Render table header with column mapping dropdowns
     * @param {number} numColumns - Number of columns to render
     * @private
     */
    _renderTableHeader(numColumns) {
        console.log('[CSVTableController] Rendering table header with', numColumns, 'columns');

        if (!this.csvTableHead) return;

        let html = '<tr>';
        for (let i = 0; i < numColumns; i++) {
            html += '<th draggable="true" data-column-index="' + i + '">';
            html +=   '<div class="' + this.config.classes.columnHeader + '">';
            html +=     '<span class="' + this.config.classes.dragHandle + '">';
            html +=       '<i class="fa ' + this.config.classes.dragHandleFaIcon + '"></i>';
            html +=     '</span>';
            html +=     '<div class="' + this.config.classes.columnMapping + '">';
            html +=       '<select class="' + this.config.classes.columnMappingSelect + '" data-column-index="' + i + '">';
            html +=         this._generateMappingOptions(i);
            html +=       '</select>';
            html +=     '</div>';
            html +=   '</div>';
            html += '</th>';
        }
        html += '</tr>';

        this.csvTableHead.innerHTML = html;

        // Bind events
        this._bindColumnDragEvents();
        this._bindMappingChangeEvents();
    }

    /**
     * Generate mapping options HTML for a column
     * @param {number} columnIndex - Column index
     * @returns {string} HTML string for options
     * @private
     */
    _generateMappingOptions(columnIndex) {
        const accountType = this.config.settings.accountType;
        const classes = this.config.classes;
        const options = this.config.data.mappingOptions || [];
        const selectedValue = this.columnMappings[columnIndex] || '---';

        let html = '';
        options.forEach((option) => {
            const selected = option.value === selectedValue ? ' selected' : '';
            html += '<option ';
            html += 'value="' + option.value + '"';
            html += selected;
            html += '>';
            html += this._escapeHtml(option.label);
            html += '</option>';
        });

        return html;
    }

    /**
     * Render table body with data rows
     * @param {Array<Array<string>>} data - CSV data
     * @param {number} numColumns - Number of columns to render
     * @param {boolean} hasHeaders - Whether first row is headers
     * @private
     */
    _renderTableBody(data, numColumns, hasHeaders) {
        console.log('[CSVTableController] Rendering table body, showing up to 10 rows');

        if (!this.csvTableBody) return;

        let html = '';
        const maxRows = Math.min(data.length, 10);

        for (let rowIndex = 0; rowIndex < maxRows; rowIndex++) {
            const row = data[rowIndex];
            const isHeaderRow = hasHeaders && rowIndex === 0;
            const rowClass = isHeaderRow ? ' class="' + this.config.classes.headerRow + '"' : '';

            html += '<tr' + rowClass + '>';

            for (let colIndex = 0; colIndex < numColumns; colIndex++) {
                const cell = colIndex < row.length ? row[colIndex] : '';
                html += '<td>' + this._escapeHtml(cell) + '</td>';
            }

            html += '</tr>';
        }

        // Add indicator if there are more rows
        if (data.length > 10) {
            const remaining = data.length - 10;
            const message = this.config.translations.moreRows.replace('%1', remaining);
            html += '<tr><td colspan="' + numColumns + '" class="' + this.config.classes.moreRowsIndicator + '">';
            html += message;
            html += '</td></tr>';
        }

        this.csvTableBody.innerHTML = html;
    }

    /**
     * Bind drag and drop events to column headers
     * @private
     */
    _bindColumnDragEvents() {
        const headers = this.csvTableHead.querySelectorAll('th[draggable="true"]');

        headers.forEach((header) => {
            header.addEventListener('dragstart', (e) => this._handleDragStart(e));
            header.addEventListener('dragover',  (e) => this._handleDragOver(e));
            header.addEventListener('dragenter', (e) => this._handleDragEnter(e));
            header.addEventListener('dragleave', (e) => this._handleDragLeave(e));
            header.addEventListener('drop',      (e) => this._handleDrop(e));
            header.addEventListener('dragend',   (e) => this._handleDragEnd(e));
        });
    }

    /**
     * Bind change events to mapping selects
     * @private
     */
    _bindMappingChangeEvents() {
        const selects = this.csvTableHead.querySelectorAll('.' + this.config.classes.columnMappingSelect);

        selects.forEach((select) => {
            select.addEventListener('change', (e) => this._handleMappingChange(e));
            // Prevent drag when clicking on select
            select.addEventListener('mousedown', (e) => e.stopPropagation());
        });
    }

    /**
     * Handle drag start event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragStart(e) {
        this.draggedColumn = parseInt(e.target.dataset.columnIndex);
        console.log('[CSVTableController] Drag started for column', this.draggedColumn);
        e.target.classList.add(this.config.classes.dragging);
        e.dataTransfer.effectAllowed = 'move';
        e.dataTransfer.setData('text/html', e.target.innerHTML);
    }

    /**
     * Handle drag over event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragOver(e) {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'move';
        return false;
    }

    /**
     * Handle drag enter event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragEnter(e) {
        e.target.classList.add(this.config.classes.dragOver);
    }

    /**
     * Handle drag leave event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragLeave(e) {
        e.target.classList.remove(this.config.classes.dragOver);
    }

    /**
     * Handle drop event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDrop(e) {
        e.stopPropagation();

        const targetColumn = parseInt(e.target.closest('th').dataset.columnIndex);

        console.log('[CSVTableController] Column dropped - from:', this.draggedColumn, 'to:', targetColumn);

        if (this.draggedColumn !== targetColumn) {
            this._swapColumns(this.draggedColumn, targetColumn);
        }

        return false;
    }

    /**
     * Handle drag end event
     * @param {DragEvent} e - Drag event
     * @private
     */
    _handleDragEnd(e) {
        const headers = this.csvTableHead.querySelectorAll('th');
        headers.forEach((header) => {
            header.classList.remove(this.config.classes.dragging);
            header.classList.remove(this.config.classes.dragOver);
        });
    }

    /**
     * Handle mapping change event
     * @param {Event} e - Change event
     * @private
     */
    _handleMappingChange(e) {
        const columnIndex = parseInt(e.target.dataset.columnIndex);
        this.columnMappings[columnIndex] = e.target.value;
        console.info('[CSVTableController] Column mappings updated:', this.columnMappings);
    }

    /**
     * Swap column mappings (not the data, only the headers)
     * @param {number} fromIndex - Source column index
     * @param {number} toIndex - Target column index
     * @private
     */
    _swapColumns(fromIndex, toIndex) {
        console.log('[CSVTableController] Swapping columns', fromIndex, 'and', toIndex);

        // Swap mappings only
        const tempMapping = this.columnMappings[fromIndex];
        this.columnMappings[fromIndex] = this.columnMappings[toIndex];
        this.columnMappings[toIndex] = tempMapping;

        console.info('[CSVTableController] Column mappings after swap:', this.columnMappings);

        // Re-render headers only
        const accountType = this.config.settings.accountType;
        const expectedColumns = this.config.data.defaultMappings.length || this.currentData[0].length;
        const displayColumns = Math.max(this.currentData[0].length, expectedColumns);

        this._renderTableHeader(displayColumns);
    }

    /**
     * Get current column mappings
     * @returns {Array<string>} Column mappings
     * @public
     */
    getColumnMappings() {
        console.log('[CSVTableController] getColumnMappings called, returning:', this.columnMappings);
        return this.columnMappings;
    }

    /**
     * Escape HTML special characters
     * @param {*} text - Text to escape
     * @returns {string} Escaped HTML string
     * @private
     */
    _escapeHtml(text) {
        if (text === null || text === undefined) return '';
        const div = document.createElement('div');
        div.textContent = String(text);
        return div.innerHTML;
    }

    /**
     * Validate configuration object
     * @param {Object} config - Configuration to validate
     * @throws {Error} If configuration is invalid
     * @private
     */
    _validateConfig(config) {
        if (!config) {
            throw new Error('[CSVTableController] Configuration object is required');
        }

        const required = ['elements', 'classes', 'translations', 'settings', 'data'];
        required.forEach((section) => {
            if (!config[section]) {
                throw new Error('[CSVTableController] config.' + section + ' is required');
            }
        });
    }
}
