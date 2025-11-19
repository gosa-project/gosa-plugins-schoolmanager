{* Phase 5 specific CSS *}
<link rel="stylesheet" href="plugins/schoolmanager/themes/default/css/content_importaccounts_phase5.css">
<link rel="stylesheet" href="plugins/schoolmanager/themes/default/css/dynamic-account-table.css">

<input type="hidden" name="phase_05">
<h3>
    {t 1="5/11"}STEP %1  (LDAP import status): Importing user account objects into LDAP{/t}
</h3>

{* Status message box - populated by JavaScript *}
<div id="import-status-box" class="alert alert-info">
    <h4><i class="fa fa-spinner fa-spin"></i> {t}Import in progress...{/t}</h4>
    <p>
        {t}User accounts are being imported to LDAP in the background. The table below shows accounts that have been processed so far.{/t}
    </p>
    {if $job_id}
    <p>
        <strong>{t}Job ID:{/t}</strong> <code>{$job_id}</code>
    </p>
    {/if}
</div>

{* Error and warning boxes (hidden by default) *}
<div id="error-box" class="alert alert-danger"></div>
<div id="warning-box" class="alert alert-warning"></div>

{* Progress bar - always rendered, JavaScript will update values *}
<div class="progress-container">
    <p><strong>{t}Progress:{/t}</strong></p>
    <p id="progress-message">
        {t}Initializing import...{/t}
    </p>
    <div id="progress-bar-wrapper">
        <div class="primary" id="progress-bar"></div>
    </div>
    <p id="progress-stats">
        <span id="progress-current">0</span> /
        <span id="progress-total">0</span>
        (<span id="progress-percent">0</span>%)
    </p>
</div>

{* Accounts table container *}
<div id="accounts-table-container">
    {* Render accounts table via JavaScript after AJAX fetch *}
    <p><em>{t}No accounts processed yet...{/t}</em></p>
</div>

{* External JavaScript - StatusMonitor class with client-side rendering *}
<script type="text/javascript" src="plugins/schoolmanager/common/js/dynamic-account-table.js"></script>
<script type="text/javascript">
(function() {
    // Initialize and start monitoring with default theme configuration
    const monitor = new StatusMonitor({
        refreshTime: {$import_job_refresh_time},
        theme: 'default',
        formData: {
            ajaxStatusCheck: { name: 'ajax_status_check', value: '1' },
            phaseField: { name: 'phase_05', value: '1' }
        },
        elements: {
            statusBox: 'import-status-box',
            errorBox: 'error-box',
            warningBox: 'warning-box',
            progressCurrent: 'progress-current',
            progressTotal: 'progress-total',
            progressPercent: 'progress-percent',
            progressBar: 'progress-bar',
            progressMessage: 'progress-message',
            accountsContainer: 'accounts-table-container',
            nextButton: 'input[name="phase_06"]'
        },
        translations: {
            processing: '{t}Processing...{/t}',
            fetchError: '{t}Failed to fetch status{/t}',
            someAccountsFailed: '{t}Some accounts failed to import. Review the error details in the table below.{/t}',
            noAccountsProcessed: '{t}No accounts processed yet...{/t}',
            userAccount: '{t 1="%1" 2="%2"}User account: %1, %2{/t}',
            newUserAccount: '{t 1="%1" 2="%2"}New user account: %1, %2{/t}',
            associatedAccount: '{t 1="%1" 2="%2"}Associated account: %1, %2{/t}',
            newAssociatedAccount: '{t 1="%1" 2="%2"}New associated account: %1, %2{/t}',
            insufficientAccountData: '{t 1="%1" 2="%2"}Insufficient account data: %1, %2{/t}',
            keep: '{t}<keep>{/t}',
            importCompleted: '{t}Import completed{/t}',
            importFailed: '{t}Import failed{/t}',
            importCompletedWithErrors: '{t}Import completed with errors{/t}',
            allAccountsImported: '{t}All user accounts have been successfully imported to LDAP.{/t}'
        }
    });

    // Make the monitor public for debugging
    window.StatusMonitor = monitor;

    monitor.start();
})();
</script>
