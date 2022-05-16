<input type="hidden" name="phase_02">
<br>
<h3>{t 1="2/11"}STEP %1: Configure import options{/t}</h3>

<p>
    {t}School Manager's import functionality is very flexible and can be adapted to
    your needs. Please configure options of this import below.{/t}
</p>

<table summary="{t}Configure import options{/t}">
    {if $import_account_type == 'studentsandparents'}
        <tr>
            <td colspan="3">
                <hr>
                <p>
                    {t}Please choose from the available GOsa2 user object templates
                    what account templates to use for students and for their
                    parents.{/t}
                </p>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="template_studentsandparents">
                    {t}Select template for student accounts{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <select id="template_studentsandparents" size="1" title=""
                        name="template_studentsandparents">
                    {html_options options=$templates selected=$preset_template_studentsandparents}
                </select>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="template_studentsandparents_aux">
                    {t}Select template for parent accounts{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <select id="template_studentsandparents_aux" size="1" title=""
                        name="template_studentsandparents_aux">
                    {html_options options=$templates selected=$preset_template_studentsandparents_aux}
                </select>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="accounts_in_class_ou">
                    {t escape=no}Create a sub-OU for each class and place student
                    accounts belonging to the same class into their corresponding
                    class OU?{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <input type="checkbox" id="accounts_in_class_ou"
                       name="accounts_in_class_ou"
                       {if $preset_accounts_in_class_ou}checked{/if}>
                {t}(tick this check box, if yes){/t}
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="domain_users">
                    {t}Mail domain for new students (if not specified in import file
                    explicitly):{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                @<input type="text" id="domain_users" name="domain_users"
                        value="{$preset_domain_users}">
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="try_mail_as_uid">
                    {t escape=no}If CSV data does not contain a user ID (uid)
                    column, use the mail address as user ID instead?{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid"
                       {if $preset_try_mail_as_uid}checked{/if}>
                {t}(tick this check box, if yes){/t}
            </td>
        </tr>
    {elseif $import_account_type == 'teachers'}
        <tr>
            <td colspan="3">
                <hr>
                {t}Please choose from the available GOsa2 user object templates what
                account template to use for teachers.{/t}
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="template_teachers">
                    {t}Select template for teacher accounts{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <select id="template_teachers" name="template_teachers"
                        size="1" title="">
                    {html_options options=$templates selected=$preset_template_teachers}
                </select>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="add_course_members_to_class_group">
                    {t escape=no}Make all teachers teaching courses in a class
                    members of that class group?{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">

            <input type="checkbox" id="add_course_members_to_class_group"
                   name="add_course_members_to_class_group"
                   {if $preset_try_mail_as_uid}checked{/if}>
            {t}(tick this check box, if yes){/t}
        </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="domain_users">
                    {t}Mail domain for new teachers (if not specified in import file
                    explicitly):{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                @
                <input type="text" id="domain_users" name="domain_users"
                       value="{$preset_domain_users}">
            </td>
        </tr>

        {if $preset_domain_school}
            <tr>
                <td style="width: 1em;">&nbsp;</td>
                <td style="vertical-align: middle;">
                    <LABEL for="aliases_in_schooldomain">
                        {t}Teachers shall have mail aliases in the school's mail
                        domain ({$preset_domain_school}):{/t}
                    </LABEL>
                </td>
                <td style="vertical-align: middle;">
                    <input type="checkbox" id="aliases_in_schooldomain"
                           name="aliases_in_schooldomain"
                {if $preset_aliases_in_schooldomain}checked{/if}>
                </td>
            </tr>
        {/if}
    {elseif $import_account_type == 'studentsonly'}
        <tr>
            <td colspan="3">
                <hr>
                <p>{t}Please choose from the available GOsa2 user object templates
                what account templates to use for students.{/t}</p>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="template_studentsonly">
                    {t}Select template for student accounts{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <select id="template_studentsonly" name="template_studentsonly"
                        size="1" title="">
                {html_options options=$templates selected=$preset_template_studentsonly}
                </select>
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="accounts_in_class_ou">
                    {t escape=no}Create a sub-OU for each class and place student
                    accounts belonging to the same class into their corresponding
                    class OU?{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <input type="checkbox" id="accounts_in_class_ou"
                       name="accounts_in_class_ou"
                       {if $preset_accounts_in_class_ou}checked{/if}>
                {t}(tick this check box, if yes){/t}
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="domain_users">
                    {t}Mail domain for new students (if not specified in import file
                    explicitly):{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                @
                <input type="text" id="domain_users" name="domain_users"
                       value="{$preset_domain_users}">
            </td>
        </tr>
        <tr>
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="try_mail_as_uid">
                    {t escape=no}If CSV data does not contain a user ID (uid) column,
                    use the mail address as user ID instead?{/t}
                </LABEL>
            </td>
            <td style="vertical-align: middle;">
                <input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid"
                       {if $preset_try_mail_as_uid}checked{/if}>
                {t}(tick this check box, if yes){/t}
            </td>
        </tr>
    {/if}

    <tr>
        <td colspan="3">
            <hr>
            <p>{t}During this import we will probably create several non-primary
            groups (for classes, courses). Where in the LDAP tree shall these new
            group objects be created?{/t}</p>
        </td>
    </tr>
    <tr>
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="ou_groups">{t}Select OU for new groups:{/t}</LABEL>
        </td>
        <td style="vertical-align: middle;">
            <select id="ou_groups" name="ou_groups" size="1" title="">
                {html_options options=$ous_available selected=$preset_ou_groups}
            </select>
        </td>
    </tr>
    <tr>
        <td colspan="3">
            <p>{t}And what mail domain should these new groups be in?{/t}</p>
        </td>
    </tr>
    <tr>
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="domain_groups">{t}Mail domain for new groups:{/t}</LABEL>
        </td>
        <td style="vertical-align: middle;">
            @<input type="text" id="domain_groups" name="domain_groups"
                    value="{$preset_domain_groups}">
        </td>
    </tr>

    <tr><td colspan="3"><table style="min-width: 100%;">
        <tr>
        <td colspan="3">
            <hr>
            <p>{t}Please select the attributes which will identify users from the
            CSV file as existing users{/t}:</p>
        </td>
    </tr>

    {if $import_account_type == 'studentsonly' ||
        $import_account_type == 'studentsandparents'}
        <tr style="background-color:whitesmoke; color: black;"> <!-- black on lightgray -->
            <td style="width: 1em;">&nbsp;</td>
            <td style="vertical-align: middle;">
                <LABEL for="sel_ldap_match_attr_studentid">{t escape=no}Student ID{/t}</LABEL>
            </td>
            <td style="float: right;">
                <input type="checkbox" id="sel_ldap_match_attr_studentid"
                       name="sel_ldap_match_attr_studentid"
                       {if $preset_sel_ldap_match_attr_studentid}checked{/if}>
                {t}(tick this check box, if yes){/t}
            </td>
        </tr>
    {/if}

    <!-- black on gray -->
    <tr style="background-color: gainsboro; color: black">
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="sel_ldap_match_attr_givenname">
                {t escape=no}Given Name{/t}
            </LABEL>
        </td>
        <td style="float: right;">
            <input type="checkbox" id="sel_ldap_match_attr_givenname"
                   name="sel_ldap_match_attr_givenname" 
                   {if $preset_sel_ldap_match_attr_givenname}checked{/if}>
        {t}(tick this check box, if yes){/t}
        </td>
    </tr>

    <!-- black on lightgray -->
    <tr style="background-color:whitesmoke; color: black;">
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="sel_ldap_match_attr_snname">{t escape=no}Surname{/t}</LABEL>
        </td>
        <td style="float: right;">
            <input type="checkbox" id="sel_ldap_match_attr_snname"
                   name="sel_ldap_match_attr_snname" 
                   {if $preset_sel_ldap_match_attr_snname}checked{/if}>
            {t}(tick this check box, if yes){/t}
        </td>
    </tr>

    <!-- black on gray -->
    <tr style="background-color: gainsboro; color: black">
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="sel_ldap_match_attr_birthday">
                {t escape=no}Date of Birth{/t}
            </LABEL>
        </td>
        <td style="float: right;">
            <input type="checkbox" id="sel_ldap_match_attr_birthday"
                   name="sel_ldap_match_attr_birthday" 
                   {if $preset_sel_ldap_match_attr_birthday}checked{/if}>
            {t}(tick this check box, if yes){/t}
        </td>
    </tr>

    <!-- black on lightgray -->
    <tr style="background-color: whitesmoke; color: black;">
        <td style="width: 1em;">&nbsp;</td>
        <td style="vertical-align: middle;">
            <LABEL for="sel_ldap_match_attr_gender">
                {t escape=no}Gender{/t}
            </LABEL>
        </td>
        <td style="float: right;">
            <input type="checkbox" id="sel_ldap_match_attr_gender"
                   name="sel_ldap_match_attr_gender" 
                   {if $preset_sel_ldap_match_attr_gender}checked{/if}>
            {t}(tick this check box, if yes){/t}
        </td>
    </tr>
</table>
