<p>
    {t}With GOsa² School Manager's {/t}<b>{t}Manage Students Module{/t}</b>{t} you can import student user accounts, parent accounts and course groups from a CSV file containing.{/t}
</p>
<p>
    {t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
</p>
<p>
    FIXME! Todo...
</p>

<hr>

<!-- PHASE 1: CSV file upload -->

{if $file_uploaded != TRUE}
<input type="hidden" name="phase1">
<table summary="{t}Upload CSV File{/t}">
	<tr>
		<td style="vertical-align: middle;">
			<LABEL for="userfile">{t}Select CSV file with student data to import:{/t}</LABEL>
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

<p>
{t}Please choose from the availble GOsa² user object templates what account templates to use for students and for their parents.{/t}
<br><br>
FIXME: Provide a checkbox to disable parent imports.
</p>
<table summary="{t}Select user object templates{/t}">
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
			<select id="template" name="template_parent" size="1" title="">
			{html_options options=$templates selected=""}
			</select>
		</td>
	</tr>
</table>


<!-- PHASE 3: CSV data sorting -->

{elseif $data_sorted != TRUE}
<input type="hidden" name="phase3">
<br>
<br>
<table summary="{t}Check CSV data{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{foreach from=$data[0] item=val key=key}
	<tr>
		<td bgcolor="#BBBBBB">
			<select name="row{$key}" size="1" title="">
				{html_options options=$attrs selected=$selectedattrs[$key]}
			</select>
		</td>
		{foreach from=$data item=val2 key=key2}
			<td bgcolor="#EEEEEE">
				{$data[$key2][$key]}&nbsp;
			</td>
		{/foreach}
	</tr>
{/foreach}
</table>

{else}

<input type="hidden" name="phase4">
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
	Continue here, when ready: <button type='submit' name='fileup'>{t}Upload CSV and Select User Templates (Step 1/4){/t}</button>
	{elseif $templates_selected != TRUE}
	Continue here, when ready: <input name="sorted" value="{t}Check and Sort CSV Data (Step 2/4){/t}" type ="submit">
	{elseif $data_sorted != TRUE}
	Continue here, when ready: <input name="sorted" value="{t}View Summary before Import (Step 3/4){/t}" type ="submit">
	{elseif $summary_checked != TRUE}
	Continue here, when ready: <input name="sorted" value="{t}Import Data into LDAP (Step 4/4){/t}" type ="submit">
	{/if}
</div>
