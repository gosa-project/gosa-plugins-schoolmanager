/**
 * StatusMonitor: Handles AJAX polling and UI updates for account import status
 *
 * This class is theme-aware and adapts its rendering based on the theme configuration.
 * It polls the server for status updates and dynamically renders account tables.
 *
 * @class StatusMonitor
 */
class StatusMonitor {
    /**
     * Creates a new StatusMonitor instance
     * @param {Object} config - Configuration object
     * @param {number} config.refreshTime - Polling interval in seconds
     * @param {Object} config.translations - Translation strings
     * @param {string} config.theme - Theme name ('classic' or 'default')
     * @param {Object} config.formData - Form data configuration (field names, values)
     * @param {Object} config.elements - DOM element IDs and classes
     */
    constructor(config) {
        this.pollInterval = config.refreshTime * 1000;
        this.pollTimer = null;
        this.isRunning = false;
        this.translations = config.translations || {};
        this.theme = config.theme || 'default';
        this.formDataConfig = config.formData || {};
        this.elements = config.elements || {};
    }

    /**
     * Starts the status monitoring by initiating AJAX polling
     */
    start() {
        this.isRunning = true;
        console.log('[StatusMonitor] Starting AJAX polling...');
        this.poll(); // Immediate first call
    }

    /**
     * Polls the server for status updates
     * @async
     */
    async poll() {
        if (!this.isRunning) return;

        try {
            // Find the main GOsa² form (contains session tokens and other required data)
            const mainForm = document.mainform || document.forms[0];
            if (!mainForm) {
                throw new Error('Main form not found');
            }

            // Copy all form data from mainform (includes GOsa² session tokens, etc.)
            const ajaxFormData = new FormData(mainForm);

            // Add configured form fields from template
            for (const key in this.formDataConfig) {
                if (this.formDataConfig.hasOwnProperty(key)) {
                    const field = this.formDataConfig[key];
                    if (field && field.name) {
                        ajaxFormData.append(field.name, field.value || '');
                    }
                }
            }

            const response = await fetch(window.location.href, {
                method: 'POST',
                body: ajaxFormData
            });

            if (!response.ok) {
                throw new Error('HTTP ' + response.status + ': ' + response.statusText);
            }

            const data = await response.json();
            console.log('[StatusMonitor] Received status:', data);

            this.updateUI(data);

            if (!data.job_running) {
                this.stop();
                this.onComplete(data);
            } else {
                // Schedule next poll
                this.pollTimer = setTimeout(() => this.poll(), this.pollInterval);
            }

        } catch (error) {
            console.error('[StatusMonitor] Poll error:', error);
            this.showError(this.translations.fetchError + ': ' + error.message);
            // Retry after interval
            this.pollTimer = setTimeout(() => this.poll(), this.pollInterval);
        }
    }

    /**
     * Updates the UI with new data from the server
     * @param {Object} data - Status data from server
     */
    updateUI(data) {
        this.updateProgressBar(data.progress);
        this.updateAccountsTable(data.accounts);
    }

    /**
     * Updates the progress bar display
     * @param {Object} progress - Progress information
     */
    updateProgressBar(progress) {
        const currentEl = document.getElementById(this.elements.progressCurrent);
        const totalEl = document.getElementById(this.elements.progressTotal);
        const percentEl = document.getElementById(this.elements.progressPercent);
        const barEl = document.getElementById(this.elements.progressBar);
        const msgEl = document.getElementById(this.elements.progressMessage);

        if (currentEl) currentEl.textContent = progress.current || 0;
        if (totalEl) totalEl.textContent = progress.total || 0;
        if (percentEl) percentEl.textContent = progress.percent || 0;
        if (barEl) barEl.style.width = (progress.percent || 0) + '%';
        if (msgEl) msgEl.textContent = progress.message || this.translations.processing;
    }

    /**
     * Updates the accounts table container with new account data
     * @param {Object} accounts - Account data from server
     */
    updateAccountsTable(accounts) {
        const container = document.getElementById(this.elements.accountsContainer);
        if (!container) return;

        container.innerHTML = this.renderAccountsTable(accounts);
    }

    /**
     * Renders the complete accounts table
     * @param {Object} accounts - Account data to render
     * @returns {string} HTML string for the accounts table
     */
    renderAccountsTable(accounts) {
        if (!accounts || Object.keys(accounts).length === 0) {
            return '<p><em>' + this.translations.noAccountsProcessed + '</em></p>';
        }

        // Get keys and reverse to show newest (most recently processed) first
        const keys = Object.keys(accounts).reverse();

        let html = '';
        keys.forEach(key => {
            html += this.renderAccountRow(accounts[key], key);
        });
        return html;
    }

    /**
     * Renders a single account row (including main and auxiliary accounts)
     * @param {Object} data - Account data
     * @param {string} key - Account key
     * @returns {string} HTML string for the account row
     */
    renderAccountRow(data, key) {
        const account = data.main_account || {};
        const auxAccounts = data.aux_accounts || [];

        const sn = this.escapeHtml(account.sn ? account.sn[0] : 'unknown');
        const givenName = this.escapeHtml(account.givenName ? account.givenName[0] : 'unknown');
        const status = account._status ? account._status[0] : 'unchecked';

        let colSpan = 2;
        if (auxAccounts.length > 0) {
            colSpan = 2 + auxAccounts.length;
        }

        let headerText = (status === 'not-found') ?
            this.translations.newUserAccount.replace('%1', sn).replace('%2', givenName) :
            this.translations.userAccount.replace('%1', sn).replace('%2', givenName);

        if (this.theme === 'classic') {
            return this.renderAccountRowClassic(account, auxAccounts, headerText, colSpan);
        } else {
            return this.renderAccountRowDefault(account, auxAccounts, headerText, colSpan);
        }
    }

    /**
     * Renders a single account row using classic theme styling
     * @param {Object} account - Main account data
     * @param {Array} auxAccounts - Auxiliary accounts data
     * @param {string} headerText - Header text for the table
     * @param {number} colSpan - Column span for header
     * @returns {string} HTML string for the account row
     */
    renderAccountRowClassic(account, auxAccounts, headerText, colSpan) {
        // Classic theme uses table-based layout with cellspacing, border, cellpadding, bgcolor
        let html = '<table cellspacing="1" border="0" cellpadding="4" bgcolor="#FEFEFE">';
        html += '<tr><td bgcolor="#BBBBBB" colspan="' + colSpan + '">' + headerText + '</td></tr>';

        // Main account section
        if (auxAccounts.length > 0) {
            html += '<tr><td><table>';
        }

        html += this.renderAccountProperties(account);

        if (auxAccounts.length > 0) {
            html += '</table></td>';

            // Auxiliary accounts
            auxAccounts.forEach((auxAccount, idx) => {
                html += '<td>' + this.renderAuxAccount(auxAccount) + '</td>';
            });
            html += '</tr>';
        }

        html += '</table><br>';

        return html;
    }

    /**
     * Renders a single account row using default theme styling
     * @param {Object} account - Main account data
     * @param {Array} auxAccounts - Auxiliary accounts data
     * @param {string} headerText - Header text for the table
     * @param {number} colSpan - Column span for header
     * @returns {string} HTML string for the account row
     */
    renderAccountRowDefault(account, auxAccounts, headerText, colSpan) {
        // Default theme uses modern HTML with Bootstrap classes
        let html = '<table class="responsive-table">';
        html += '<thead><tr><th colspan="' + colSpan + '">' + headerText + '</th></tr></thead>';
        html += '<tbody>';

        // Main account section
        if (auxAccounts.length > 0) {
            html += '<tr><td><table>';
        }

        html += this.renderAccountProperties(account);

        if (auxAccounts.length > 0) {
            html += '</table></td>';

            // Auxiliary accounts
            auxAccounts.forEach((auxAccount, idx) => {
                html += '<td>' + this.renderAuxAccount(auxAccount) + '</td>';
            });
            html += '</tr>';
        }

        html += '</tbody></table>';

        return html;
    }

    /**
     * Renders the properties of an account as table rows
     * Aligned with accounts_table.tpl rendering logic
     * @param {Object} account - Account data
     * @returns {string} HTML string for account properties
     */
    renderAccountProperties(account) {
        let html = '';

        for (const property in account) {
            if (!account.hasOwnProperty(property)) continue;

            const value = account[property];
            if (!Array.isArray(value)) continue;

            if (this.theme === 'classic') {
                html += '<tr><td bgcolor="#EEEEEE"><b>' + this.escapeHtml(property) + ':</b></td>';
                html += '<td bgcolor="#F8F8F8">';
            } else {
                html += '<tr><td><b>' + this.escapeHtml(property) + ':</b></td><td>';
            }

            // Special handling for certain properties (aligned with accounts_table.tpl)
            if (property === 'userPassword') {
                const status = account._status ? account._status[0] : '';
                // Check if value is empty string
                if (value[0] === '') {
                    html += ''; // Empty
                } else if (status.indexOf('exists') !== -1) {
                    html += this.translations.keep;
                } else if (status.indexOf('data-incomplete') !== -1) {
                    html += '';
                } else {
                    html += '************';
                }
            } else if (property === 'alias') {
                // Render all aliases except the 'count' property (PHP LDAP arrays include 'count')
                Object.keys(value).forEach((key) => {
                    if (key !== 'count' && typeof value[key] === 'string') {
                        html += this.escapeHtml(value[key]) + '<br>';
                    }
                });
            } else if (property === '_error') {
                // Allow HTML tags in error messages (e.g., <b>, <i>, <br>)
                html += value[0] || '';
            } else {
                // Default: render first value
                html += this.escapeHtml(value[0] || '');
            }

            html += '</td></tr>';
        }

        return html;
    }

    /**
     * Renders an auxiliary account
     * @param {Object} auxAccount - Auxiliary account data
     * @returns {string} HTML string for auxiliary account
     */
    renderAuxAccount(auxAccount) {
        const sn = this.escapeHtml(auxAccount.sn ? auxAccount.sn[0] : 'unknown');
        const givenName = this.escapeHtml(auxAccount.givenName ? auxAccount.givenName[0] : 'unknown');
        const status = auxAccount._status ? auxAccount._status[0] : 'unchecked';

        let headerText = '';
        if (status === 'not-found') {
            headerText = this.translations.newAssociatedAccount.replace('%1', sn).replace('%2', givenName);
        } else if (status === 'data-incomplete') {
            headerText = this.translations.insufficientAccountData.replace('%1', sn).replace('%2', givenName);
        } else {
            headerText = this.translations.associatedAccount.replace('%1', sn).replace('%2', givenName);
        }

        let html = '';
        if (this.theme === 'classic') {
            html = '<table cellspacing="1" border="0" cellpadding="4" bgcolor="#FEFEFE">';
            html += '<tr><td bgcolor="#BBBBBB" colspan="2">' + headerText + '</td></tr>';
            html += this.renderAccountProperties(auxAccount);
            html += '</table>';
        } else {
            html = '<table>';
            html += '<thead><tr><th colspan="2">' + headerText + '</th></tr></thead>';
            html += '<tbody>';
            html += this.renderAccountProperties(auxAccount);
            html += '</tbody></table>';
        }

        return html;
    }

    /**
     * Handles completion of the import job
     * @param {Object} data - Final status data from server
     */
    onComplete(data) {
        console.log('[StatusMonitor] Import completed');

        const statusBox = document.getElementById(this.elements.statusBox);
        if (!statusBox) return;

        if (this.theme === 'classic') {
            this.onCompleteClassic(statusBox, data);
        } else {
            this.onCompleteDefault(statusBox, data);
        }

        // Enable "Next Phase" button
        const nextBtn = document.querySelector(this.elements.nextButton);
        if (nextBtn) {
            nextBtn.disabled = false;
        }
    }

    /**
     * Handles completion using classic theme styling
     * @param {HTMLElement} statusBox - The status box element
     * @param {Object} data - Final status data
     */
    onCompleteClassic(statusBox, data) {
        const contentCell = statusBox.querySelector('.alert-content-cell');
        if (!contentCell) return;

        if (data.error) {
            statusBox.className = 'alert-table alert-table-danger';
            const iconCell = statusBox.querySelector('.alert-icon-cell');
            if (iconCell) iconCell.innerHTML = '<i class="fa fa-times-circle fa-2x"></i>';
            contentCell.innerHTML = '<h4>' + this.translations.importFailed +
                '</h4><p>' + this.escapeHtml(data.error) + '</p>';
        } else if (data.has_failures) {
            statusBox.className = 'alert-table alert-table-warning';
            const iconCell = statusBox.querySelector('.alert-icon-cell');
            if (iconCell) iconCell.innerHTML = '<i class="fa fa-exclamation-triangle fa-2x"></i>';
            contentCell.innerHTML = '<h4>' +
                this.translations.importCompletedWithErrors + '</h4><p>' +
                this.translations.someAccountsFailed + '</p>';
        } else {
            statusBox.className = 'alert-table alert-table-success';
            const iconCell = statusBox.querySelector('.alert-icon-cell');
            if (iconCell) iconCell.innerHTML = '<i class="fa fa-check-circle fa-2x"></i>';
            contentCell.innerHTML = '<h4>' +
                this.translations.importCompleted + '</h4><p>' +
                this.translations.allAccountsImported + '</p>';
        }
    }

    /**
     * Handles completion using default theme styling
     * @param {HTMLElement} statusBox - The status box element
     * @param {Object} data - Final status data
     */
    onCompleteDefault(statusBox, data) {
        if (data.error) {
            statusBox.className = 'alert alert-danger';
            statusBox.innerHTML = '<h4><i class="fa fa-times-circle"></i> ' +
                this.translations.importFailed + '</h4><p>' +
                this.escapeHtml(data.error) + '</p>';
        } else if (data.has_failures) {
            statusBox.className = 'alert alert-warning';
            statusBox.innerHTML = '<h4><i class="fa fa-exclamation-triangle"></i> ' +
                this.translations.importCompletedWithErrors + '</h4><p>' +
                this.translations.someAccountsFailed + '</p>';
        } else {
            statusBox.className = 'alert alert-success';
            statusBox.innerHTML = '<h4><i class="fa fa-check-circle"></i> ' +
                this.translations.importCompleted + '</h4><p>' +
                this.translations.allAccountsImported + '</p>';
        }
    }

    /**
     * Displays an error message
     * @param {string} message - Error message to display
     */
    showError(message) {
        const errorBox = document.getElementById(this.elements.errorBox);
        if (!errorBox) return;

        if (this.theme === 'classic') {
            errorBox.classList.add('visible');
            const td = errorBox.querySelector('.alert-content-cell');
            if (td) td.textContent = message;
        } else {
            errorBox.classList.add('visible');
            errorBox.textContent = message;
        }
    }

    /**
     * Displays a warning message
     * @param {string} message - Warning message to display
     */
    showWarning(message) {
        const warnBox = document.getElementById(this.elements.warningBox);
        if (!warnBox) return;

        if (this.theme === 'classic') {
            warnBox.classList.add('visible');
            const td = warnBox.querySelector('.alert-content-cell');
            if (td) td.textContent = message;
        } else {
            warnBox.classList.add('visible');
            warnBox.textContent = message;
        }
    }

    /**
     * Escapes HTML characters to prevent XSS
     * @param {*} text - Text to escape
     * @returns {string} Escaped HTML string
     */
    escapeHtml(text) {
        if (text === null || text === undefined) return '';
        const div = document.createElement('div');
        div.textContent = String(text);
        return div.innerHTML;
    }

    /**
     * Stops the status monitoring
     */
    stop() {
        this.isRunning = false;
        if (this.pollTimer) {
            clearTimeout(this.pollTimer);
            this.pollTimer = null;
        }
        console.log('[StatusMonitor] Stopped polling');
    }
}
