<template>
	<view class="app-header">
		<view class="header-glow"></view>
		<view class="header-content">
			<view>
				<text class="header-kicker">{{ kicker }}</text>
				<text class="header-title">{{ title }}</text>
				<text v-if="subtitle" class="header-subtitle">{{ subtitle }}</text>
			</view>
			<slot name="right"></slot>
		</view>
	</view>
</template>

<script>
export default {
	name: 'AppHeader',
	props: {
		title: {
			type: String,
			required: true
		},
		subtitle: {
			type: String,
			default: ''
		},
		kicker: {
			type: String,
			default: 'IoT Control'
		}
	}
}
</script>

<style>
.app-header {
	position: relative;
	margin: -32rpx -32rpx 28rpx;
	padding: 78rpx 32rpx 38rpx;
	border-radius: 0 0 44rpx 44rpx;
	background: linear-gradient(135deg, #EAF3FF 0%, #E7FBF7 54%, #F8FAFC 100%);
	overflow: hidden;
}

.header-glow {
	position: absolute;
	right: -80rpx;
	top: -80rpx;
	width: 260rpx;
	height: 260rpx;
	border-radius: 50%;
	background: rgba(37, 99, 235, 0.12);
}

.header-content {
	position: relative;
	z-index: 1;
	display: flex;
	align-items: flex-end;
	justify-content: space-between;
	gap: 20rpx;
}

.header-kicker {
	display: block;
	margin-bottom: 8rpx;
	font-size: 22rpx;
	font-weight: 750;
	color: #14B8A6;
	letter-spacing: 0;
}

.header-title {
	display: block;
	font-size: 44rpx;
	line-height: 1.15;
	font-weight: 900;
	color: #0F172A;
}

.header-subtitle {
	display: block;
	margin-top: 10rpx;
	font-size: 24rpx;
	color: #64748B;
}
</style>
