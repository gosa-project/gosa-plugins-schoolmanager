<p>
    {if $import_account_type == "students"}
    {t escape=no}With GOsa2 School Manager's <b>Manage Students Module</b> you can import student user accounts, parent accounts and course groups from a CSV file containing.{/t}
    {elseif $import_account_type == "teachers"}
    {t escape=no}With GOsa2 School Manager's <b>Manage Teachers Module</b> you can import teacher user accounts and course groups from a CSV file containing.{/t}
    {/if}
</p>
<hr>

<!-- PHASE 1: CSV file upload -->

{if $file_uploaded != TRUE}
<input type="hidden" name="phase1">

<br><h3>{t}STEP 1: Upload CSV File{/t}</h3>

<p>
    {t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
</p>
<p>
    FIXME! Todo...
</p>

<table summary="{t}Upload CSV File{/t}">
    <tr>
	<td style="vertical-align: middle;">
	    <LABEL for="userfile">{t}Select CSV file with {$import_account_type} data to import:{/t}</LABEL>
	</td>
	<td style="vertical-align: middle;">
	    <input type="hidden" name="MAX_FILE_SIZE" value="2097152">
	    <input id="userfile" name="userfile" type="file" value="{t}Browse{/t}">
	</td>
    </tr>
</table>


<!-- PHASE 2: Selecting templates -->

{elseif $templates_selected != TRUE}
<input type="hidden" name="phase2">

<br><h3>{t}STEP 2: Select user object templates{/t}</h3>

{if $import_account_type == 'students'}
    <p>
    {t}Please choose from the available GOsa2 user object templates what account templates to use for students and for their parents.{/t}
    <br><br>
    FIXME: Provide a checkbox to disable parent imports.
    </p>
{/if}
{if $import_account_type == 'teachers'}
    <p>
    {t}Please choose from the available GOsa2 user object templates what account template to use for teachers.{/t}
    </p>
{/if}


<table summary="{t}Select user object templates{/t}">
    {if $import_account_type == 'students'}
    <tr>
	<td style="vertical-align: middle;">
	    <LABEL for="template">{t}Select template for student accounts{/t}</LABEL>
	</td>
	<td style="vertical-align: middle;">
	    <select id="template" name="template_students" size="1" title="">
	    {html_options options=$templates selected=""}
	    </select>
	</td>
    </tr>
    <tr>
	<td style="vertical-align: middle;">
	    <LABEL for="template">{t}Select template for parent accounts{/t}</LABEL>
	</td>
	<td style="vertical-align: middle;">
	    <select id="template" name="template_students_aux" size="1" title="">
	    {html_options options=$templates selected=""}
	    </select>
	</td>
    </tr>
    {/if}
    {if $import_account_type == 'teachers'}
    <tr>
	<td style="vertical-align: middle;">
	    <LABEL for="template">{t}Select template for teacher accounts{/t}</LABEL>
	</td>
	<td style="vertical-align: middle;">
	    <select id="template" name="template_teachers" size="1" title="">
	    {html_options options=$templates selected=""}
	    </select>
	</td>
    </tr>
    {/if}
</table>


<!-- PHASE 3: CSV data sorting -->

{elseif $data_sorted != TRUE}
<input type="hidden" name="phase3">

<br><h3>{t}STEP 3: Check CSV data and assign to LDAP attributes{/t}</h3>

<p>
{t}Please assign the offered (LDAP) attributes to the CSV data columns. Note that the CSV file has been rotated 90DEGREES counter-clockwise for better readability.{/t}
</p>
<br>
<br>
<table summary="{t}Check CSV data{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{foreach from=$data[0] item=val key=key}
    <tr>
	<td bgcolor="#BBBBBB">
	    <select name="column_head_{$key}" size="1" title="">
		{html_options options=$attrs selected=$attrs_selected[$key]}
	    </select>
	</td>
	{foreach from=$data item=val2 key=key2}
	    <td bgcolor="#EEEEEE">
		{$data[$key2][$key]}&nbsp;
	    </td>
	{/foreach}
	{if $key == 0}
	<td  style="vertical-align: middle;" bgcolor="#EEEEEE" rowspan={$num_rows}>&nbsp;&nbsp;&nbsp;<b>...</b>&nbsp;&nbsp;&nbsp;</td>
	{/if}
    </tr>
{/foreach}
</table>

{elseif $groups_reviewed != TRUE || $groups_imported != TRUE}
{if $groups_reviewed}
<input type="hidden" name="phase4_status">
{else}
<input type="hidden" name="phase4">
{/if}

{if $groups_reviewed}
<br><h3>{t}STEP 4 (LDAP import status): Group objects have been imported into LDAP{/t}</h3>
{else}
<br><h3>{t}STEP 4: Review group objects before LDAP import{/t}</h3>
{/if}
<br>
<br>
{foreach from=$data item=group key=key}
<div style="float:left; width:20em;">
<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
	<tr>
	<td bgcolor="#BBBBBB" colspan="2">
		{t 1=$key}Group object: %1{/t}
	</td>
	</tr>
	{foreach from=$group item=value key=property}
	<tr>
		<td bgcolor="#EEEEEE">
		<b>{$property}:</b>
		</td>
		<td bgcolor="#FEFEFE">
		{$group[$property][0]}
		</td>
        </tr>
	{/foreach}
</table>
<br>
</div>
<div style="float:left; width:1em;">&nbsp;</div>
{/foreach}
<div class="clear"></div>


{elseif $accounts_reviewed != TRUE || $accounts_imported != TRUE}
{if $accounts_reviewed}
<input type="hidden" name="phase5_status">
{else}
<input type="hidden" name="phase5">
{/if}

{if $accounts_reviewed}
<br><h3>{t}STEP 5 (LDAP import status): User account objects have been imported into LDAP{/t}</h3>
{else}
<br><h3>{t}STEP 5: Review user account objects before LDAP import{/t}</h3>
{/if}
<br>
<br>
{foreach from=$data item=row key=key}
<table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}New account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">

	<tr>
	{if $data[$key]['aux_accounts']}
	<td bgcolor="#BBBBBB" colspan={2 + count($data[$key]['aux_accounts'])}>
	{else}
	<td bgcolor="#BBBBBB" colspan="2">
	{/if}
		{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}New account: %1, %2{/t}
	</td>
	</tr>

	{if $data[$key]['aux_accounts']}
	<tr>
	<td>
	<table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}Account group for %1, %2{/t}"
	{/if}
	{foreach from=$data[$key]['main_account'] item=value key=property}
	<tr>
		<td bgcolor="#EEEEEE">
		<b>{$property}:</b>
		</td>
		<td bgcolor="#FEFEFE">
		{$data[$key]['main_account'][$property][0]}
		</td>
	</tr>
	{/foreach}
	{if $data[$key]['aux_accounts']}
	</table>
	{foreach $data[$key]['aux_accounts'] item=aux_account key=idx_aux_account}
	<td>
		<table summary="{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}New account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
			<tr>
			<td bgcolor="#BBBBBB" colspan="2">
				{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}New associated account: %1, %2{/t}
			</td>
			</tr>

			{foreach from=$aux_account item=value key=property}
			<tr>
				<td bgcolor="#EEEEEE">
				<b>{$property}:</b>
				</td>
				<td bgcolor="#FEFEFE">
				{$aux_account[$property][0]}
			</td>
			</tr>
			{/foreach}
		</table>
	</td>
	{/foreach}
	</tr>
	</table>
	</td>
	</tr>
	{/if}
</table>
{/foreach}

{else}
<input type="hidden" name="phase5">

<br>

{if $error == FALSE}
<b>{t}All entries have been written to the LDAP database successfully.{/t}</b>
{else}
<b style="color:red">{t}There was an error during the import of your data.{/t}</b>
{/if}

<b>{t}Here is the status report for the import:{/t} </b>
<br>
<br>
    <table summary="{t}Status report{/t}" cellspacing="1" border=0 cellpadding="4"  bgcolor="#FEFEFE">
	{foreach from=$head item=h}
	<tr>
	    <td bgcolor="#BBBBBB">
		<b>{$h}</b>
	    </td>
	    {foreach from=$data item=row key=key}
	    <td bgcolor="#EEEEEE">
		{$data[$key][$key2]}
	    </td>
	    {/foreach}
	</tr>
    {/foreach}
    </table>
{/if}


<!-- SUBMIT: Different buttons depending on the above phases -->

<hr>
<div class="plugin-actions">
    {if $file_uploaded != TRUE}
    {t}Continue here, when ready{/t}: <button type='submit' name='fileup'>{t}Select User Templates (Step 2/7){/t}</button>
    {elseif $templates_selected != TRUE}
    {t}Continue here, when ready{/t}: <input name="btn_template_selected" value="{t}Check and Sort CSV Data (Step 3/7){/t}" type ="submit">
    {elseif $data_sorted != TRUE}
    {t}Continue here, when ready{/t}: <input name="btn_data_sorted" value="{t}Review group objects (Step 4/7){/t}" type ="submit">
    {elseif $groups_reviewed != TRUE}
    {t}Continue here, when ready{/t}: <input name="btn_data_sorted" value="{t}Import group objects into LDAP (Step 5/7){/t}" type ="submit">
    {elseif $groups_imported != TRUE}
    {t}Continue here, when ready{/t}: <input name="btn_data_sorted" value="{t}Review user account objects (Step 6/7){/t}" type ="submit">
    {elseif $accounts_reviewed != TRUE}
    {t}Continue here, when ready{/t}: <input name="btn_accounts_reviewed" value="{t}Import user account objects into LDAP (Step 7/7){/t}" type ="submit">
    {/if}
</div>
