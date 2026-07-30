<link rel="stylesheet" type="text/css" href="plugins/schoolmanager/themes/default/css/content_importaccounts_phase1.css">
<script type="text/javascript" src="plugins/schoolmanager/common/js/content_importaccounts_phase1.js"></script>

<script type="text/javascript">
// Configuration for FileUploadController and CSVTableController
// All element IDs, classes, strings, and settings are defined here
window.phase1Config = {
    // DOM Element IDs
    elements: {
        dropZone: 'dropZone',
        dropZoneIcon: 'dropZoneIcon',
        dropZoneTitle: 'dropZoneTitle',
        dropZoneSubtitle: 'dropZoneSubtitle',
        fileInput: 'userfile',
        errorMessage: 'errorMessage',
        delimiterWarning: 'delimiterWarning',
        csvPreview: 'csvPreview',
        csvWithHeaders: 'csv_with_column_headers',
        customDelimiterRadio: 'custom_delimiter_radio',
        downloadTemplateBtn: 'downloadTemplateBtn',
        btnQuickImport: 'btnQuickImport',
        btnContinue: 'btnContinue'
    },

    // CSS Class Names
    classes: {
        fileUploadedFaIcon: 'fa-check-circle',
        active: 'active',
        dragover: 'dragover',
        hasFile: 'has-file',
        open: 'open',
        headerRow: 'header-row',
        moreRowsIndicator: 'more-rows-indicator'
    },

    // CSS Selectors for Querying
    selectors: {
        delimiterRadios: 'input[name="delimiter_id"]',
        collapsibleSections: '.collapsible-section',
        collapsibleHeader: '.collapsible-header'
    },

    // Translated Strings (from Smarty t-tags)
    translations: {
        invalidFileType: "{t}Invalid file type. Please select a CSV file (.csv extension required).{/t}",
        fileTooLarge: "{t}File size exceeds %1 MB limit. Please select a smaller file.{/t}",
        readError: "{t}Error reading file. Please try again.{/t}",
        emptyFile: "{t}CSV file appears to be empty.{/t}",
        moreRows: "{t}... and %1 more row(s){/t}"
    },

    // Business Logic Settings
    settings: {
        maxFileSize: 2097152,
        allowedExtensions: ['.csv'],
        accountType: "{$import_account_type}"
    },

    // Data Structures
    data: {
        // CSV Templates for download
        csvTemplates: {
            studentsandparents: {
                headers: ['{t}Number{/t}', '{t}Login{/t}', '{t}Password{/t}', '{t}Student ID{/t}', '{t}Last Name(s){/t}', '{t}First Name(s){/t}', '{t}Date of Birth{/t}', '{t}Gender{/t}', '{t}Class{/t}', '{t}Last Name (Parent 1){/t}', '{t}First Name (Parent 1){/t}', '{t}Mail Address (Parent 1){/t}', '{t}Last Name (Parent 2){/t}', '{t}First Name (Parent 2){/t}', '{t}Mail Address (Parent 2){/t}', '{t}Course{/t}', '{t}Course{/t}'],
                examples: [
                    ['1', 'jdoe', 'password123', '12345', 'Doe', 'John', '2010-05-15', 'male', '5A', 'Doe', 'Jane', 'jane.doe@example.com', 'Doe', 'Jack', 'jack.doe@example.com', 'Math', 'Science'],
                    ['2', 'asmith', '', '12346', 'Smith', 'Alice', '2010-08-22', 'female', '5A', 'Smith', 'Mary', 'mary.smith@example.com', 'Smith', 'Bob', 'bob.smith@example.com', 'English', 'Art']
                ]
            },
            teachers: {
                headers: ['{t}Number{/t}', '{t}Login{/t}', '{t}Password{/t}', '{t}Last Name{/t}', '{t}First Name{/t}', '{t}Date of Birth{/t}', '{t}Gender{/t}', '{t}Mail Address{/t}', '{t}Subjects{/t}', '{t}Class{/t}', '{t}Course{/t}', '{t}Course{/t}'],
                examples: [
                    ['1', 'mjohnson', 'teacher123', 'Johnson', 'Michael', '1980-03-10', 'male', 'michael.johnson@school.edu', 'Math, Physics', '5A', 'Math-Grade5', 'Physics-Grade6'],
                    ['2', 'swilliams', '', 'Williams', 'Sarah', '1985-07-20', 'female', 'sarah.williams@school.edu', 'English, History', '', 'English-Grade5', 'History-Grade6']
                ]
            },
            studentsonly: {
                headers: ['{t}Number{/t}', '{t}Login{/t}', '{t}Password{/t}', '{t}Student ID{/t}', '{t}Last Name(s){/t}', '{t}First Name(s){/t}', '{t}Date of Birth{/t}', '{t}Gender{/t}', '{t}Class{/t}', '{t}Course{/t}', '{t}Course{/t}'],
                examples: [
                    ['1', 'jdoe', 'password123', '12345', 'Doe', 'John', '2010-05-15', 'male', '5A', 'Math', 'Science'],
                    ['2', 'asmith', '', '12346', 'Smith', 'Alice', '2010-08-22', 'female', '5A', 'English', 'Art']
                ]
            }
        },

        // Column mapping options per account type
        mappingOptions: [
        {foreach $attributes_all as $value=>$label}
            { value: '{$value}', label: '{$label}' },
        {/foreach}
            { value: '---', label: '{t}(Ignore Column){/t}' },
        ],

        // Default column mappings (order matters)
        // Uses 'value' from mappingOptions (excluding 'ignore')
        defaultMappings: [
            {foreach $attributes_all as $value=>$label}
                '{$value}',
            {/foreach}
        ],

        // Attributes that can have multiple entries (e.g., courses)
        multiAttributeOptions: [
            {foreach $attributes_multi as $value}
                '{$value}',
            {/foreach}
        ],
    },

    // Sub-configuration for CSVTableController
    table: {
        elements: {
            mainForm: 'mainform',
            csvTable: 'csvTable',
            csvTableHead: 'csvTableHead',
            csvTableBody: 'csvTableBody',
            statRows: 'statRows',
            statColumns: 'statColumns',
        },

        classes: {
            dragHandleFaIcon: 'fa-ellipsis-v',
            columnHeader: 'column-header',
            dragHandle: 'drag-handle',
            columnMapping: 'column-mapping',
            columnMappingSelect: 'column-mapping-select',
            dragging: 'dragging',
            dragOver: 'drag-over',
            headerRow: 'header-row',
            moreRowsIndicator: 'more-rows-indicator',
        },

        translations: {
            moreRows: "{t}... and %1 more row(s){/t}",
        },

        settings: {
            accountType: "{$import_account_type}",
            postVarColumnMappings: 'column_head_' // + column $index
        },

        data: {
            mappingOptions:        {},  // To be filled down below
            defaultMappings:       {},  // To be filled down below
            multiAttributeOptions: {},  // To be filled down below
        }
    }
};

// Fix circular reference for table config
if (window.phase1Config) {
    window.phase1Config.table.data.mappingOptions = window.phase1Config.data.mappingOptions;
    window.phase1Config.table.data.defaultMappings = window.phase1Config.data.defaultMappings;
    window.phase1Config.table.data.multiAttributeOptions = window.phase1Config.data.multiAttributeOptions;
}

// Initialize FileUploadController on DOM ready
document.addEventListener('DOMContentLoaded', function() {
    try {
        window.phase1Instance = new FileUploadController(window.phase1Config);
        console.log('[Phase1] Initialized successfully');
        // CSVTableController will be initialized by FileUploadController after file upload
    } catch (error) {
        console.error('[Phase1] Initialization failed:', error);
    }
});
</script>

<input type="hidden" name="phase_01">

<h3>{t 1="1/10"}STEP %1: Upload CSV File{/t}</h3>

<!-- Collapsible Column Format Section -->
<div class="collapsible-section" id="columnFormatSection">
    <div class="collapsible-header">
        <h4>{t}Expected CSV Column Format{/t}</h4>
        <span class="collapsible-toggle">▼</span>
    </div>
    <div class="collapsible-content">
        <p>
            {t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
        </p>

        <table class="column-table">
    {if $import_account_type == "studentsandparents"}
        <tr>
            <td>{t}Column{/t} 01</td>
            <td><b>{t}Number{/t}</b></td>
            <td>{t}This is useful for discussing issues with CSV import files, but the number column is normally not used.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 02</td>
            <td><b>{t}Login{/t}</b></td>
            <td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 03</td>
            <td><b>{t}Password{/t}</b></td>
            <td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 04</td>
            <td><b>{t}Student ID{/t}</b></td>
            <td>{t}An unequivocal number that a student is associated with in the school's administration software.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 05</td>
            <td><b>{t}Last Name(s){/t}</b></td>
            <td>{t}The student's last name(s).{/t}</td>
            </tr>
        <tr>
            <td>{t}Column{/t} 06</td>
            <td><b>{t}First Name(s){/t}</b></td>
            <td>{t}The student's first and middle name(s).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 07</td>
            <td><b>{t}Date of Birth{/t}</b></td>
            <td>{t}The student's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 08</td>
            <td><b>{t}Gender{/t}</b></td>
            <td>{t}The student's gender (male / female / ...).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 09</td>
            <td><b>{t}Class{/t}</b></td>
            <td>{t}The class name / identifier.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 10</td>
            <td><b>{t}Last Name (Parent 1){/t}</b></td>
            <td>{t}Last name of parent 1 (e.g. mother).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 11</td>
            <td><b>{t}First Name (Parent 1){/t}</b></td>
            <td>{t}First (and middle) name(s) of parent 1.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 12</td>
            <td><b>{t}Mail Address (Parent 1){/t}</b></td>
            <td>{t}Mail address of parent 1.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 13</td>
            <td><b>{t}Last Name (Parent 2){/t}</b></td>
            <td>{t}Last name of parent 2 (e.g. father).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 14</td>
            <td><b>{t}First Name (Parent 2){/t}</b></td>
            <td>{t}First (and middle) name(s) of parent 2.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 15</td>
            <td><b>{t}Mail Address (Parent 2){/t}</b></td>
            <td>{t}Mail address of parent 2.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 16&ndash;XX</td>
            <td><b>{t}Courses{/t}</b></td>
            <td>{t}Courses a student is signed up for (one per column, use as many columns as needed).{/t}</td>
        </tr>
    {elseif $import_account_type == "teachers"}
        <tr>
            <td>{t}Column{/t} 01</td>
            <td><b>{t}Number{/t}</b></td>
            <td>{t}This is useful for manual discussing issues with CSV import files, but the number column is normally not used.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 02</td>
            <td><b>{t}Login{/t}</b></td>
            <td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 03</td>
            <td><b>{t}Password{/t}</b></td>
            <td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 04</td>
            <td><b>{t}Last Name{/t}</b></td>
            <td>{t}The teacher's last name(s).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 05</td>
            <td><b>{t}First Name{/t}</b></td>
            <td>{t}The teacher's first (and middle) name(s).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 06</td>
            <td><b>{t}Date of Birth{/t}</b></td>
            <td>{t}The teachers's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 07</td>
            <td><b>{t}Gender{/t}</b></td>
            <td>{t}The teacher's gender (male / female / ...).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 08</td>
            <td><b>{t}Mail Address{/t}</b></td>
            <td>{t}The teacher's mail address.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 09</td>
            <td><b>{t}Subjects{/t}</b></td>
            <td>{t}The subjects a teacher can teach / is trained for.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 10</td>
            <td><b>{t}Class{/t}</b></td>
            <td>{t}The primary class a teacher is assigned to. If the teacher is not a class teacher, then leave empty.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 11&ndash;XX</td>
            <td><b>{t}Courses{/t}</b></td>
            <td>{t}Courses a teacher teaches (one per column, use as many columns as needed).{/t}</td>
        </tr>
    {elseif $import_account_type == "studentsonly"}
        <tr>
            <td>{t}Column{/t} 01</td>
            <td><b>{t}Number{/t}</b></td>
            <td>{t}This is useful for discussing issues with CSV import files, but the number column is normally not used.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 02</td>
            <td><b>{t}Login{/t}</b></td>
            <td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 03</td>
            <td><b>{t}Password{/t}</b></td>
            <td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 04</td>
            <td><b>{t}Student ID{/t}</b></td>
            <td>{t}An unequivocal number that a student is associated with in the school's administration software.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 05</td>
            <td><b>{t}Last Name(s){/t}</b></td>
            <td>{t}The student's last name(s).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 06</td>
            <td><b>{t}First Name(s){/t}</b></td>
            <td>{t}The student's first and middle name(s).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 07</td>
            <td><b>{t}Date of Birth{/t}</b></td>
            <td>{t}The student's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 08</td>
            <td><b>{t}Gender{/t}</b></td>
            <td>{t}The student's gender (male / female / ...).{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 09</td>
            <td><b>{t}Class{/t}</b></td>
            <td>{t}The class name / identifier.{/t}</td>
        </tr>
        <tr>
            <td>{t}Column{/t} 10&ndash;XX</td>
            <td><b>{t}Courses{/t}</b></td>
            <td>{t}Courses a student is signed up for (one per column, use as many columns as needed).{/t}</td>
        </tr>
    {/if}
        </table>

        <button type="button" class="download-template-btn" id="downloadTemplateBtn">
            {t}Download CSV Template{/t}
        </button>
    </div>
</div>

<!-- CSV Format Options -->
<div class="csv-format-options section">
    <h4>{t}CSV format options:{/t}</h4>

    <div style="margin: 10px 0;">
        <label for="csv_with_column_headers">
        {if $preset_csv_with_column_headers}
            <input type="checkbox" id="csv_with_column_headers" name="csv_with_column_headers" checked>
        {else}
            <input type="checkbox" id="csv_with_column_headers" name="csv_with_column_headers">
        {/if}
            <span>{t}First line in CSV file contains column headers (so ignore that first line){/t}</span>
        </label>
    </div>

    <!-- Delimiter Selection -->
    <div style="margin: 15px 0 0 0;">
        <label style="font-weight: bold; display: block; margin-bottom: 8px;">
            {t}Delimiter for column fields in CSV file:{/t}
        </label>
        <div class="delimiter-options">
            {foreach $delims_texts as $id=>$delimiter_text}
            <label>
                <input type="radio" name="delimiter_id" value="{$id}" data-delimiter="{$delims_chars[$id]}"{if $id == $delims_default_key} checked{/if}{if $id == 4} id="custom_delimiter_radio"{/if}>
                <span>{$delimiter_text}</span>
            </label>
            {/foreach}
        </div>
    </div>
</div>

<!-- Drag and Drop Upload Area -->
<div class="drop-zone" id="dropZone">
    <div class="drop-zone-icon" id="dropZoneIcon"><i class="fa fa-file"></i></div>
    <div class="drop-zone-text">
        <strong id="dropZoneTitle">{t}Drag & Drop CSV file here or click to browse{/t}</strong>
        <div class="drop-zone-subtitle" id="dropZoneSubtitle">{t}Supported format: .csv (max 2MB){/t}</div>
    </div>
    <input type="file" id="userfile" name="userfile" accept=".csv" class="file-input-hidden">
    <input type="hidden" name="MAX_FILE_SIZE" value="2097152">
</div>

<div class="error-message" id="errorMessage"></div>
<div class="warning-message" id="delimiterWarning">
    <strong>{t}Warning:{/t}</strong>
    {t escape=no}Only one column detected in the CSV file. This usually means the delimiter is <strong>incorrect</strong>.{/t}
    {t escape=no}Please check the "CSV format options" setting above and select the correct delimiter used in your CSV file.{/t}
    {t escape=no}You can also change the <strong>default delimiter</strong> in the <strong>settings</strong> menu.{/t}
</div>

<!-- CSV Preview -->
<div class="csv-preview" id="csvPreview">
    <h4>{t}CSV Preview:{/t}</h4>
    <div class="csv-stats">
        <div class="stat-item">
            <div class="stat-value" id="statRows">0</div>
            <div class="stat-label">{t}Rows{/t}</div>
        </div>
        <div class="stat-item">
            <div class="stat-value" id="statColumns">0</div>
            <div class="stat-label">{t}Columns{/t}</div>
        </div>
    </div>
    <div class="csv-table-wrapper">
        <table class="csv-table" id="csvTable">
            <thead id="csvTableHead">
                <!-- Column mapping headers will be inserted here by CSVTableController -->
            </thead>
            <tbody id="csvTableBody">
                <!-- CSV data rows will be inserted here by CSVTableController -->
            </tbody>
        </table>
    </div>
</div>

<div class="note-box">
    <b>{t}Note:{/t}</b> {t}The following filters are automatically applied on CSV file import...{/t}
    <ul>
        <li>{t}Auto-detection of charset encoding (everything gets converted internally to UTF-8){/t}</li>
        <li>{t}Single and double quotes next to the field delimiters get removed{/t}</li>
        <li>{t}MS-DOS / Unix line ending style gets recognized automatically and processed accordingly{/t}</li>
    </ul>
</div>
