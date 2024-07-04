<input type="hidden" name="phase_02">

<h3>{t 1="2/11"}STEP %1: Configure import options{/t}</h3>

<p>
    {t}School Manager's import functionality is very flexible and can be adapted to
    your needs. Please configure options of this import below.{/t}
</p>

<hr class="divider">

<div class="section">
{if $import_account_type == 'studentsandparents'}
    <p>
        {t}Please choose from the available GOsa2 user object templates
        what account templates to use for students and for their
        parents.{/t}
    </p>
    <div class="input-field">
        <label for="template_studentsandparents">
            {t}Select template for student accounts{/t}
        </label>
        <select id="template_studentsandparents" size="1" title=""
                name="template_studentsandparents">
            {foreach $templates as $id=>$template}
                <option value="{$id}"{if $id == $preset_template_studentsandparents} selected{/if}>{$template}</option>
            {/foreach}
        </select>
    </div>
    <div class="input-field">
        <label for="template_studentsandparents_aux">
            {t}Select template for parent accounts{/t}
        </label>
        <select id="template_studentsandparents_aux" size="1" title=""
                name="template_studentsandparents_aux">
            {foreach $templates as $id=>$template}
                <option value="{$id}"{if $id == $preset_template_studentsandparents_aux} selected{/if}>{$template}</option>
            {/foreach}
        </select>
    </div>
    <div>
        <label>
            <input type="checkbox" id="accounts_in_class_ou"
                   name="accounts_in_class_ou"
                   {if $preset_accounts_in_class_ou}checked{/if}>
            <span>{t escape=no}Create a sub-OU for each class and place student
            accounts belonging to the same class into their corresponding
            class OU?{/t} {t}(tick this check box, if yes){/t}</span>
        </label>
    </div>
    <div class="input-field">
        <input type="text" id="domain_users" name="domain_users"
                value="{$preset_domain_users}">
        <label for="domain_users">
            {t}Mail domain for new students (if not specified in import file
            explicitly):{/t}
        </label>
    </div>
    <div>
        <label>
            <input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid"
                   {if $preset_try_mail_as_uid}checked{/if}>
            <span>{t escape=no}If CSV data does not contain a user ID (uid)
            column, use the mail address as user ID instead?{/t}
            {t}(tick this check box, if yes){/t}</span>
        </label>
    </div>
{elseif $import_account_type == 'teachers'}
    <p>{t}Please choose from the available GOsa2 user object templates what
        account template to use for teachers.{/t}</p>
    <div class="input-field">
        <label for="template_teachers">
            {t}Select template for teacher accounts{/t}
        </label>
        <select id="template_teachers" name="template_teachers"
                size="1" title="">
            {foreach $templates as $id=>$template}
                <option value="{$id}"{if $id == $preset_template_teachers} selected{/if}>{$template}</option>
            {/foreach}
        </select>
    </div>
    <div>
        <label>
            <input type="checkbox" id="add_course_members_to_class_group"
                   name="add_course_members_to_class_group"
                   {if $preset_try_mail_as_uid}checked{/if}>
            <span>{t escape=no}Make all teachers teaching courses in a class
            members of that class group?{/t}
            {t}(tick this check box, if yes){/t}</span>
        </label>
    </div>
    <div class="input-field">
        <input type="text" id="domain_users" name="domain_users"
               value="{$preset_domain_users}">
        <label for="domain_users">
            {t}Mail domain for new teachers (if not specified in import file
            explicitly):{/t}
        </label>
    </div>

    {if $preset_domain_school}
        <div>
            <label>
                <input type="checkbox" id="aliases_in_schooldomain"
                       name="aliases_in_schooldomain"
            {if $preset_aliases_in_schooldomain}checked{/if}>
                <span>{t}Teachers shall have mail aliases in the school's mail domain ({$preset_domain_school}):{/t}</span>
            </label>
        </div>
    {/if}
{elseif $import_account_type == 'studentsonly'}
    <p>{t}Please choose from the available GOsa2 user object templates
    what account templates to use for students.{/t}</p>
    <div class="input-field">
        <label for="template_studentsonly">
            {t}Select template for student accounts{/t}
        </label>
        <select id="template_studentsonly" name="template_studentsonly"
                size="1" title="">
            {foreach $templates as $id=>$template}
                <option value="{$id}"{if $id == $preset_template_studentsonly} selected{/if}>{$template}</option>
            {/foreach}
        </select>
    </div>
    <div>
        <label>
            <input type="checkbox" id="accounts_in_class_ou"
                   name="accounts_in_class_ou"
                   {if $preset_accounts_in_class_ou}checked{/if}>
            <span>{t escape=no}Create a sub-OU for each class and place student
                accounts belonging to the same class into their corresponding
                class OU?{/t} {t}(tick this check box, if yes){/t}</span>
        </label>
    </div>
    <div class="input-field">
        <input type="text" id="domain_users" name="domain_users"
               value="{$preset_domain_users}">
        <label for="domain_users">
            {t}Mail domain for new students (if not specified in import file
            explicitly):{/t}
        </label>
    </div>
    <div>
        <label>
            <input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid"
                   {if $preset_try_mail_as_uid}checked{/if}>
            </span>{t escape=no}If CSV data does not contain a user ID (uid) column,
                use the mail address as user ID instead?{/t}
                {t}(tick this check box, if yes){/t}</span>
        </label>
    </div>
{/if}
</div>

<hr class="divider">

<p>{t}During this import we will probably create several non-primary
groups (for classes, courses). Where in the LDAP tree shall these new
group objects be created?{/t}</p>
<div class="input-field">
    <select id="ou_groups" name="ou_groups" size="1" title="">
        {foreach $ous_available as $id=>$ou}
            <option value="{$id}"{if $id == $preset_ou_groups} selected{/if}>{$ou}</option>
        {/foreach}
    </select>
    <label for="ou_groups">{t}Select OU for new groups:{/t}</label>
</div>
<p>{t}And what mail domain should these new groups be in?{/t}</p>
<div class="input-field">
    <input type="text" id="domain_groups" name="domain_groups"
            value="{$preset_domain_groups}">
    <label for="domain_groups">{t}Mail domain for new groups:{/t}</label>
</div>

<hr class="divider">

<p>{t}Please select the attributes which will identify users from the
CSV file as existing users{/t}:</p>

{if $import_account_type == 'studentsonly' ||
    $import_account_type == 'studentsandparents'}
    <div>
        <label>
            <input type="checkbox" id="sel_ldap_match_attr_studentid"
                   name="sel_ldap_match_attr_studentid"
                   {if $preset_sel_ldap_match_attr_studentid}checked{/if}>
            <span>{t escape=no}Student ID{/t} {t}(tick this check box, if yes){/t}</span>
        </label>
    <div>
{/if}
<div>
    <label>
        <input type="checkbox" id="sel_ldap_match_attr_givenname"
               name="sel_ldap_match_attr_givenname" 
               {if $preset_sel_ldap_match_attr_givenname}checked{/if}>
        <span>{t escape=no}Given Name{/t} {t}(tick this check box, if yes){/t}</span>
    </label>
</div>
<div>
    <label>
        <input type="checkbox" id="sel_ldap_match_attr_snname"
               name="sel_ldap_match_attr_snname" 
               {if $preset_sel_ldap_match_attr_snname}checked{/if}>
        <span>{t escape=no}Surname{/t} {t}(tick this check box, if yes){/t}</span>
    </label>
</div>
<div>
    <label>
        <input type="checkbox" id="sel_ldap_match_attr_birthday"
               name="sel_ldap_match_attr_birthday" 
               {if $preset_sel_ldap_match_attr_birthday}checked{/if}>
        <span>{t escape=no}Date of Birth{/t} {t}(tick this check box, if yes){/t}</span>
    </label>
</div>
<div>
    <label>
        <input type="checkbox" id="sel_ldap_match_attr_gender"
               name="sel_ldap_match_attr_gender" 
               {if $preset_sel_ldap_match_attr_gender}checked{/if}>
        <span>{t escape=no}Gender{/t} {t}(tick this check box, if yes){/t}</span>
    </label>
</div>
