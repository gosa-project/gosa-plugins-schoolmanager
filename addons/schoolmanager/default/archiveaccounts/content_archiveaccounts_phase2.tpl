<input type="hidden" name="phase_02">

<h3>{t 1="3/11"}STEP %1: Configure archive options{/t}</h3>

<p>
{t}School Manager's archive functionality is very flexible and can be adapted to your needs. Please configure options of this archiving below.{/t}
</p>
<hr class="divider">

<p>{t}Where in the LDAP tree should the archived Users go?{/t}</p>

<div class="input-field">
    <label for="ou_users">
        {t}Select OU for archiving users:{/t}
    </label>
    <select id="ou_users" name="ou_users" size="1" title="">
        {foreach $ous_available as $id=>$ou}
            <option value="{$id}"{if $id == $preset_ou_users} selected{/if}>{$ou}</option>
        {/foreach}
    </select>
</div>

<hr class="divider">

<p>{t}Where in the LDAP tree should we look for matching Users?{/t}</p>

<div class="input-field">
    <label for="ou_matching_users">
        {t}Select OU for archiving users:{/t}
    </label>
    <select id="ou_matching_users" name="ou_matching_users" size="1" title="">
        {foreach $ous_available as $id=>$ou}
            <option value="{$id}"{if $id == $preset_ou_matching_users} selected{/if}>{$ou}</option>
        {/foreach}
    </select>
</div>

<hr class="divider">

<p>{t}Please select the attributes which will identify users from the CSV file as existing users{/t}</p>

{if $archive_type === "students"}
<label>
{if $preset_sel_ldap_match_attr_studentid}
    <input type="checkbox" id="sel_ldap_match_attr_studentid" name="sel_ldap_match_attr_studentid" checked>
{else}
    <input type="checkbox" id="sel_ldap_match_attr_studentid" name="sel_ldap_match_attr_studentid">
{/if}
    <span>{t escape=no}Student ID{/t} {t}(tick this check box, if yes){/t}</span>
</label>
{/if}

<label>
{if $preset_sel_ldap_match_attr_name}
    <input type="checkbox" id="sel_ldap_match_attr_name" name="sel_ldap_match_attr_name" checked>
{else}
    <input type="checkbox" id="sel_ldap_match_attr_name" name="sel_ldap_match_attr_name">
{/if}
<span>{t escape=no}Name{/t} {t}(tick this check box, if yes){/t}</span>
</label>

<label>
{if $preset_sel_ldap_match_attr_snname}
    <input type="checkbox" id="sel_ldap_match_attr_snname" name="sel_ldap_match_attr_snname" checked>
{else}
    <input type="checkbox" id="sel_ldap_match_attr_snname" name="sel_ldap_match_attr_snname">
{/if}
<span>{t escape=no}Second Name{/t} {t}(tick this check box, if yes){/t}</span>
</label>

<label>
{if $preset_sel_ldap_match_attr_birthday}
    <input type="checkbox" id="sel_ldap_match_attr_birthday" name="sel_ldap_match_attr_birthday" checked>
{else}
    <input type="checkbox" id="sel_ldap_match_attr_birthday" name="sel_ldap_match_attr_birthday">
{/if}
<span>{t escape=no}Birthday{/t} {t}(tick this check box, if yes){/t}</span>
</label>

<label>
{if $preset_sel_ldap_match_attr_gender}
    <input type="checkbox" id="sel_ldap_match_attr_gender" name="sel_ldap_match_attr_gender" checked>
{else}
    <input type="checkbox" id="sel_ldap_match_attr_gender" name="sel_ldap_match_attr_gender">
{/if}
<span>{t escape=no}Gender{/t} {t}(tick this check box, if yes){/t}</span>
</label>
