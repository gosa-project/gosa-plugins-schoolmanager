<input type="hidden" name="phase_01">
<br>
<h3>{t 1="1/11"}STEP %1: Upload CSV File{/t}</h3>

<p>
    {t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
</p>

<p>
    <table>
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
                <td>{t}The student's gender (male / female / ...).{/t}</td>
            </tr>
            <tr>
                <td>{t}Column{/t} 08</td>
                <td><b>{t}Subjects{/t}</b></td>
                <td>{t}The subjects a teacher can teach / is trained for.{/t}</td>
            </tr>
            <tr>
                <td>{t}Column{/t} 09</td>
                <td><b>{t}Class{/t}</b></td>
                <td>{t}The primary class a teacher is assigned to. If the teacher is not a class teacher, then leave empty.{/t}</td>
            </tr>
            <tr>
                <td>{t}Column{/t} 10&ndash;XX</td>
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
</p>

<table summary="{t}Upload CSV File{/t}">
    <tr>
        <td style="vertical-align: middle;">
            {if $import_account_type=="studentsandparents"}
                <LABEL for="userfile">{t}Select CSV file with students' and their parents' data to import:{/t}</LABEL>
            {elseif $import_account_type=="teachers"}
            <LABEL for="userfile">{t}Select CSV file with teachers' data to import:{/t}</LABEL>
            {elseif $import_account_type=="studentsonly"}
            <LABEL for="userfile">{t}Select CSV file with students' data to import:{/t}</LABEL>
            {/if}
        </td>
        <td style="vertical-align: middle;">
            <input type="hidden" name="MAX_FILE_SIZE" value="2097152">
            <input id="userfile" name="userfile" type="file" value="{t}Browse{/t}">
        </td>
    </tr>

    <tr>
        <td>
            <h4>{t}CSV format options:{/t}</h4>
        </td>
    </tr>
    <tr>
        <td style=vertical-align: middle;">
            <LABEL for="csv_with_column_headers">{t}First line in CSV file contains column headers (so ignore that first
                line):{/t}</LABEL>
            {if $preset_csv_with_column_headers}
            <input type="checkbox" id="csv_with_column_headers" name="csv_with_column_headers" checked>
            {else}
            <input type="checkbox" id="csv_with_column_headers" name="csv_with_column_headers">
            {/if}
        </td>
    </tr>
    <tr>
        <td style=vertical-align: middle;">
            <br />
            <LABEL for="delimiter_id">{t}Delimiter for column fields in CSV file:{/t}</LABEL>
            <br />
            {html_radios name="delimiter_id" options=$available_delimiters selected=$preset_delimiter_id separator='<br />'}
        </td>
    </tr>
    <tr>
        <td>
            <br />
            <b>{t}Note:{/t}</b> {t}The following filters are automatically applied on CSV file import...{/t}
            <ul>
                <li>{t}Auto-detection of charset encoding (everything gets converted internally to UTF-8){/t}</li>
                <li>{t}Single and double quotes next to the field delimiters get removed{/t}</li>
                <li>{t}MS-DOS / Unix line ending style gets recognized automatically and processed accordingly{/t}</li>
            </ul>
        </td>
    </tr>
</table>
